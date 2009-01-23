#include "template_store.h"

#include <string>
#include <fcntl.h>
#include <unistd.h>

#include <cstdarg>
#include <math.h>

#include "templates.h"

typedef struct {
  const char  name[32];
  byte        type;
  ulong       size;
} tv_map_t;

static tv_map_t TV_MAP[] = {
  {  "__first__",       TV_FIRST,        9  },
  {  "__last__",        TV_LAST,         8  },
  {  "__odd__",         TV_ODD,          7  },
  {  "__even__",        TV_EVEN,         8  }
};

char* TNT_CONST_NAMES[] = {
  "TNT_TEXT",
  "TNT_SIMPLE",
  "TNT_INCLUDE",
  "TNT_INCLUDE_UNLESS",
  "TNT_IF",
  "TNT_UNLESS",
  "TNT_ELSE",
  "TNT_LOOP",
  "TNT_ENUM",
  "TNT_ENUM_ITEM",
  "TNT_ENUM_LAST"
};

template_store::template_store(char *fn, long sz, CTmpl *rt) {
  root = rt;
  mtm = 0;
  fname = (char*) malloc(sz+1);
  memcpy(fname, fn, sz);
  *(fname+sz) = 0;
  body = template_store::load(fname, mtm, body_size);
  max = body+body_size;
  time(&last_check);
  
  comp_struct = buffer = errors = NULL;
  compiled = NULL;
  
  compile_template();
}

template_store::~template_store() {
  clear();
  if (errors != NULL) delete errors;
  if (buffer != NULL) delete buffer;
  if (comp_struct != NULL) delete comp_struct;
  if (body != NULL) free(body);
  free(fname);
}
void template_store::clear() {
  if (compiled == NULL) return;
  template_store::free_compiled(compiled);
  compiled = NULL;
  if (comp_struct != NULL) delete comp_struct;
  comp_struct = NULL;
}
void template_store::free_compiled(tmpl_node_t *tl) {
  tmpl_node_t *f = tl;
  while(tmpl_node_t *b=f) {
    if (f->block) template_store::free_compiled(f->block);
    f = b->next;
    free(b);
  };
}
char *template_store::load(const char *fname, time_t &mtm, long &size) {

  struct stat f_stat = {0};
  struct flock my_lock = {0};
  
  int fd = 0;
  if ( (fd = open(fname, O_RDONLY, 0)) > 0 ) {
    if (!fstat(fd, &f_stat)) {
      
      if (mtm==0 || f_stat.st_mtime>mtm) {
        mtm = f_stat.st_mtime;
        char *buf = (char*) malloc(f_stat.st_size+1);

        memset(buf, 0, f_stat.st_size+1);
        
        my_lock.l_type = F_RDLCK;
        my_lock.l_whence = SEEK_SET;
        my_lock.l_start = 0;
        my_lock.l_len = 0;
        fcntl(fd, F_SETLKW, &my_lock);
    
        size = read(fd, buf, f_stat.st_size);

        my_lock.l_type = F_UNLCK;
        fcntl(fd, F_SETLKW, &my_lock);
        
        close(fd);

        if (size != f_stat.st_size) {
          free(buf);
          return NULL;
        }
        return buf;
      }
    }
    close(fd);
  }
  return NULL;
}

byte template_store::check_value_type(const char *src, ulong size) {
  byte res = TV_NORMAL;
  for (uint i=0;i<sizeof(TV_MAP)/sizeof(TV_MAP[0]); i++) {
    if (TV_MAP[i].size == size && (strncmp(TV_MAP[i].name, src, size) ==0) )
      return TV_MAP[i].type;
  }
  return res;
}

void template_store::compile_template() {
  char *src = comp_helper = body;
  src = comp_genblock(src, NULL, &compiled);
  if (src != max) comp_error(NULL, "TEMPLATE CRITICAL: unknow error, check template syntax!");
}

char* template_store::comp_genblock(const char *gsrc, const char *tag, tmpl_node_t**tnode) {
  if (gsrc==NULL || gsrc > max) return NULL;
  
  char *src = (char*)gsrc;
  while (src != NULL && src < max) {
    if (char *cur = strchr(src, '#')) {
      cur++;
      if (tag != NULL && index(tag, *cur)) {
        if (cur-1 > comp_helper)
          src = comp_helper = (char*)comp_static(cur-1, tnode);
        return cur;
      } else if (*cur == '|') {
        src = comp_helper = (char*)comp_static(cur, tnode) + 1;
      } else if (*cur == '#') {
        src = comp_helper = comp_simple(cur + 1, tnode);
      } else if (*cur == '!') {
        src = comp_helper = comp_ifelse(cur + 1, tnode);
      } else if (*cur == '@') {
        src = comp_helper = comp_loop(cur+1, tnode);
      } else if (*cur == '%') {
        src = comp_helper = comp_enum(cur+1, tnode);
      } else if (*cur == '^') {
        src = comp_helper = comp_include(cur+1, tnode);
      } else {
        src = cur;
      }
    } else {
      src = comp_helper = (char*)comp_static(max, tnode);
      break;
    }
  }
  return src;
}

void template_store::comp_error(const char *src, const char *format, ...) {

  long line, pos;
  template_store::find_line_pos(body, src, line, pos);
  
  char ddd[MAX_SNPRINTF_LENGTH];
  va_list	ap;
  va_start(ap, format);
  std::vsnprintf(ddd, MAX_SNPRINTF_LENGTH, format, ap);
  va_end(ap);

  if (errors == NULL) errors = new UStr(MIN_STRING_REALLOC);
  errors->catsnprintf("%s (%ld,%ld) %s\n", fname, line, pos, ddd);

}

const char* template_store::comp_static(const char *src, tmpl_node_t **&tnode) {
  *tnode = (tmpl_node_t*) malloc(sizeof(tmpl_node_t));
  memset(*tnode, 0, sizeof(tmpl_node_t));
  (*tnode)->type = TNT_TEXT;
  (*tnode)->pre_src = comp_helper;
  (*tnode)->pre_src_len = src-comp_helper;
  tnode = &(*tnode)->next;
  return src;
}

char* template_store::comp_simple(const char *src, tmpl_node_t **&tnode) {
  char *cur;
  if ((cur = strstr(src, "##")) != NULL) {
    *tnode = (tmpl_node_t*) malloc(sizeof(tmpl_node_t));
    memset(*tnode, 0, sizeof(tmpl_node_t));
    (*tnode)->pre_src = comp_helper;
    (*tnode)->pre_src_len = src-2-comp_helper;
    (*tnode)->type = TNT_SIMPLE;
    (*tnode)->value = (char*)src;
    (*tnode)->value_len = cur - (char*)src;
    (*tnode)->value_type = check_value_type(src, (*tnode)->value_len);
    tnode = &(*tnode)->next;
  } else {
    comp_error(src, "TEMPLATE WARNNING: can't find pair to VAR tag for '##'");
    cur = (char*)src;
  }
  return cur+2;

}

char* template_store::comp_ifelse(const char *gsrc, tmpl_node_t **&tnode) {
  char *src = (char*)gsrc;
  if (char *cur = strstr(src, "#?")) {
    bool unless = (*src == '!') ? true : false;
    if (unless) src++;
    
    char *token = src;
    int tlen = cur - token;
    
    *tnode = (tmpl_node_t*) malloc(sizeof(tmpl_node_t));
    memset(*tnode, 0, sizeof(tmpl_node_t));
    (*tnode)->pre_src = comp_helper;
    (*tnode)->pre_src_len = (char*)gsrc-2-comp_helper;
    (*tnode)->type = (unless)?TNT_UNLESS:TNT_IF;
    (*tnode)->value = (char*)token;
    (*tnode)->value_len = tlen;
    (*tnode)->value_type = check_value_type(token, tlen);

    comp_helper = src = cur+2;
    src = comp_genblock(src, ":", &(*tnode)->block) + 1;

    tnode = &(*tnode)->next;
    *tnode = (tmpl_node_t*) malloc(sizeof(tmpl_node_t));
    memset(*tnode, 0, sizeof(tmpl_node_t));
    (*tnode)->type = TNT_ELSE;
    comp_helper = src;
    src = comp_genblock(src, "$", &(*tnode)->block) + 1;
    
    tnode = &(*tnode)->next;
  } else
    comp_error(src, "TEMPLATE WARNNING: can't find pair for IF tag for '#?'");

  return src;
}

char* template_store::comp_loop(const char *gsrc, tmpl_node_t **&tnode) {
  char *src = (char*)gsrc;
  if (char *cur = strstr(src, "#:")) {
    char *token = src;
    int tlen = cur - src;
    *tnode = (tmpl_node_t*) malloc(sizeof(tmpl_node_t));
    memset(*tnode, 0, sizeof(tmpl_node_t));
    (*tnode)->pre_src = comp_helper;
    (*tnode)->pre_src_len = src-2-comp_helper;
    (*tnode)->type = TNT_LOOP;
    (*tnode)->value = token;
    (*tnode)->value_len = tlen;
    (*tnode)->value_type = check_value_type(token, tlen);
    comp_helper = src = cur+2;
    src = comp_genblock(src, "$", &(*tnode)->block) + 1;
    tnode = &(*tnode)->next;
  } else
    comp_error(src, "TEMPLATE WARNNING: can't find pair for LOOP tag for '#:'");
  return src;
}

char* template_store::comp_enum(const char *gsrc, tmpl_node_t **&tnode) {
  char *src = (char*)gsrc;
  if (char *cur = strstr(src, "#?")) {
    char *token = src;
    int tlen = cur - src;
    *tnode = (tmpl_node_t*) malloc(sizeof(tmpl_node_t));
    memset(*tnode, 0, sizeof(tmpl_node_t));
    (*tnode)->pre_src = comp_helper;
    (*tnode)->pre_src_len = src-2-comp_helper;
    (*tnode)->type = TNT_ENUM;
    (*tnode)->value = token;
    (*tnode)->value_len = tlen;
    (*tnode)->value_type = check_value_type(token, tlen);
    tmpl_node_t *root = *tnode;
    src = cur+2;
    tnode = &(*tnode)->next;
    tmpl_node_t **last;
    do {
      *tnode = (tmpl_node_t*) malloc(sizeof(tmpl_node_t));
      memset(*tnode, 0, sizeof(tmpl_node_t));
      (*tnode)->type = TNT_ENUM_ITEM;
      last = tnode;
      comp_helper = src;
      if ((cur = comp_genblock(src, ":$",  &(*tnode)->block)) == NULL) {
        tnode = &(*tnode)->next;
        comp_error(src, "TEMPLATE WARNNING: ENUM tag '%*.*s' can't find pair tag #: or #$", tlen, tlen, token);
        return src;
      }
      src = cur+1;
      tnode = &(*tnode)->next;
      root->enum_records++;
    } while (*cur != '$');
    (*last)->type = TNT_ENUM_LAST;
  } else
    comp_error(src, "TEMPLATE WARNNING: can't find pair for ENUM tag for '#?'");
  return src;
}

char* template_store::comp_include(const char *gsrc, tmpl_node_t **&tnode) {
  char *src = (char*) gsrc;
  if (char *cur = strstr(src, "#:")) {
    bool unless = (*src == '!') ? true : false;
    if (unless) src++;
    
    char *token = src;
    int tlen = cur - src;
    
    src = cur+2;
    
    if (char *cur = strstr(src, "#$")) {
      *tnode = (tmpl_node_t*) malloc(sizeof(tmpl_node_t));
      memset(*tnode, 0, sizeof(tmpl_node_t));
      (*tnode)->pre_src = comp_helper;
      (*tnode)->pre_src_len = gsrc-2-comp_helper;
      (*tnode)->type = (unless)? TNT_INCLUDE_UNLESS : TNT_INCLUDE;
      (*tnode)->value = token;
      (*tnode)->value_len = tlen;
      (*tnode)->value_type = check_value_type(token, tlen);
      tmpl_node_t **block;
      block = &(*tnode)->block;
      *block = (tmpl_node_t*) malloc(sizeof(tmpl_node_t));
      memset(*block, 0, sizeof(tmpl_node_t));
      (*block)->value = src;
      (*block)->value_len = cur-src;
      tnode = &(*tnode)->next;
      src = cur+2;
    } else
      comp_error(src, "TEMPLATE WARNNING: can't find pair for INCLUDE tag for '#$");
  } else
    comp_error(src, "TEMPLATE WARNNING: can't find pair for INCLUDE tag for '#:'");
  return src;
}UStr* template_store::get_compiled_struct() {
  if (comp_struct != NULL) return comp_struct;
  comp_struct = new UStr;
  if (compiled != NULL) r_get_compiled_struct(comp_struct, 0, compiled);
  return comp_struct;
}

void template_store::r_get_compiled_struct(UStr *dst, long lvl, tmpl_node_t *tnode) {
  do {
    dst->catsnprintf("%*.*s%s{\n", lvl*4, lvl*4, "  ", TNT_CONST_NAMES[tnode->type]);
    if (tnode->pre_src != NULL && tnode->pre_src_len >0) {
      UStr tmp(tnode->pre_src, tnode->pre_src+tnode->pre_src_len, true);
      long line, pos;
      template_store::find_line_pos(body, tnode->pre_src, line, pos);
      dst->catsnprintf("%*.*s  PRE %p,%ld [%d,%d](%s)\n", lvl*4, lvl*4, "  ", tnode->pre_src, tnode->pre_src_len, line, pos, tmp.str());
    }
    if (tnode->value != NULL && tnode->value_len >0)
      dst->catsnprintf("%*.*s  KEY %p,%ld(%*.*s)\n", lvl*4, lvl*4, "  ", tnode->value, tnode->value_len, (tnode->value_len>20)?20l:tnode->value_len, (tnode->value_len>20)?20:tnode->value_len, tnode->value);
    if (tnode->value_type > 0)
      dst->catsnprintf("%*.*s  VT (%s)%d\n", lvl*4, lvl*4, "  ", TV_MAP[tnode->value_type-1].name,tnode->value_type);
    if (tnode->block != NULL) r_get_compiled_struct(dst, lvl+1, tnode->block);
    dst->catsnprintf("%*.*s}\n", lvl*4, lvl*4, "  ");
    tnode = tnode->next;
  } while (tnode != NULL);
}

void template_store::find_line_pos(const char *src, const char *cur, long &line, long &pos) {
  line = 1;
  pos = 0;
  while (src < cur) {
    if (*src++ == '\n') { line++; pos = 0; } else pos++;
  }
}

SV** template_store::get_variable(char *name, I32 len, list_vars *vars) {
  SV** res = NULL;
  for (int i = vars->cur-1; i >=0 && res == NULL; --i) {
    res = hv_fetch(vars->vars[i], name, len, 0);
  }
  if (res == NULL) {
    for (uint i=0;i<sizeof(TV_MAP)/sizeof(TV_MAP[0]); i++) {
      if (vars->special_vars[i] != NULL && TV_MAP[i].size == (unsigned)len && (strncmp(TV_MAP[i].name, name, len) ==0)) {
        res = &vars->special_vars[TV_MAP[i].type];
        break;
      }
    }
  }
  return res;
}

bool template_store::chk_variable(char *token, I32 tlen, SV **value) {
  bool orf = false;
  if (value != NULL) {
    SV *tmp = ( SvROK(*value) ) ? SvRV(*value) : *value;
    if (SvTYPE(tmp) == SVt_NULL) {
      orf = false;
    } else if (SvTYPE(tmp) == SVt_IV
    || SvTYPE(tmp) == SVt_PVIV
    || SvTYPE(tmp) == SVt_NV
    || SvTYPE(tmp) == SVt_PVNV
    || SvTYPE(tmp) == SVt_PV
    || SvTYPE(tmp) == SVt_RV
    ) {
      orf = SvTRUE(tmp);
      SvPV_nolen(tmp);
    } else if (SvTYPE(tmp) == SVt_PVAV) {
      orf = (av_len((AV*) tmp) >= 0) ? true : false;
    } else if (SvTYPE(tmp) == SVt_PVHV) {
      orf = (HvKEYS(tmp) > 0) ? true : false;
    } else if (SvTYPE(tmp) == SVt_PVGV) {
      orf = false;
    } else if (SvTYPE(tmp) == SVt_PVMG) {
      SvPV_nolen(tmp);
      orf = SvTRUE(tmp);
    } else {
      orf = SvTRUE(tmp);
    }
  } else if (CGI_RANDOM_PARAMETER_LEN==tlen && (strncmp(CGI_RANDOM_PARAMETER, token, tlen) == 0)) {
    orf = (drand48() >= 0.5f) ? true : false;
  }
  return orf;
}

UStr* template_store::gen_template(list_vars &vars) {
  
  if (buffer == NULL) {
    long max_len = ((unsigned)body_size > PAGE_SIZE*2) ? body_size + PAGE_SIZE*8 : body_size * 2;
    max_len = ((max_len / PAGE_SIZE) + 1) * PAGE_SIZE;
    buffer = new UStr(max_len);
  }
  buffer->clear();
  time_t t1;
  time(&t1);
  if (last_check + TS_CHECK_UPDATE < t1) {
    time(&last_check);
    char *body_new = template_store::load(fname, mtm, body_size);
    if (body_new != NULL) {
      free(body);
      body = body_new;
      max = body+body_size;
      clear();
      compile_template();
    }
  }
  
  template_store::gen_template_recursive(buffer, compiled, vars);
  
  return buffer;
}

void template_store::gen_template_recursive(UStr *dst, tmpl_node_t *tnode, list_vars &vars) {

  while (tnode != NULL) {
    dst->ncat(tnode->pre_src, tnode->pre_src_len);
    switch (tnode->type) {
      case TNT_TEXT:
      case TNT_ELSE:
      case TNT_ENUM_ITEM:
      case TNT_ENUM_LAST:
        break;
      case TNT_SIMPLE: {
        SV** tmp = template_store::get_variable(tnode->value, tnode->value_len, &vars);
        if (tmp != NULL) {
          if (!SvROK(*tmp)) {
            STRLEN val_sz;
            char *val = SvPV(*tmp, val_sz);
            dst->ncat(val, val_sz);
          }
        } else {
          if (CGI_RANDOM_PARAMETER_LEN==tnode->value_len && (strncmp(CGI_RANDOM_PARAMETER, tnode->value, tnode->value_len) == 0))
            dst->catsnprintf("%.8f", drand48());
        }
        break;
      }
      case TNT_IF:
      case TNT_UNLESS: {
        SV** tmp = template_store::get_variable(tnode->value, tnode->value_len, &vars);
        bool orf = template_store::chk_variable(tnode->value, tnode->value_len, tmp);
        orf = (tnode->type==TNT_UNLESS) ? !orf : orf;
        if (orf) {
          template_store::gen_template_recursive(dst, tnode->block, vars);
        } else {
          tnode = tnode->next;
          if (tnode == NULL) return;
          template_store::gen_template_recursive(dst, tnode->block, vars);
        }
        break;
      }
      case TNT_LOOP: {
        SV** tmp = template_store::get_variable(tnode->value, tnode->value_len, &vars);
        if (tmp != NULL) {
          if (SvROK(*tmp) && SvTYPE(SvRV(*tmp)) == SVt_PVAV) {
            I32 len = av_len((AV*) SvRV(*tmp))+1;
            for (I32 i = 1; i <= len; ++i) {
              SV** vprms = av_fetch((AV*) SvRV(*tmp), i-1, 0);
              if (SvROK(*vprms) && SvTYPE(SvRV(*vprms)) == SVt_PVHV) {
                vars.push((HV*) SvRV(*vprms));
                vars.special_vars[TV_FIRST] = (i==1)?&PL_sv_yes:&PL_sv_no;
                vars.special_vars[TV_LAST] = (i==len)?&PL_sv_yes:&PL_sv_no;
                vars.special_vars[TV_ODD] = (i%2==1)?&PL_sv_yes:&PL_sv_no;
                vars.special_vars[TV_EVEN] = (i%2==0)?&PL_sv_yes:&PL_sv_no;
                template_store::gen_template_recursive(dst, tnode->block, vars);
                vars.pop();
              }
            }
          }
        }
        break;
      }
      case TNT_ENUM: {
        SV** tmp = template_store::get_variable(tnode->value, tnode->value_len, &vars);
        I32 INDEX = -1;
        if (tmp != NULL) {
          SV *value = ( SvROK(*tmp) ) ? SvRV(*tmp) : *tmp;
          INDEX = SvIV(value);
        } else if (CGI_RANDOM_PARAMETER_LEN==tnode->value_len && (strncmp(CGI_RANDOM_PARAMETER, tnode->value, tnode->value_len) == 0))
          INDEX = (int)((double)(floor(tnode->enum_records * drand48())));
        if (INDEX >= 0) {
          I32 I = 0;
          bool f = true;
          while (tnode->type != TNT_ENUM_LAST && (tnode = tnode->next) != NULL && (tnode->type == TNT_ENUM_ITEM || tnode->type == TNT_ENUM_LAST)) {
            if (tnode->type == TNT_ENUM_LAST) INDEX = I;
            if (f && I++ == INDEX) {
              template_store::gen_template_recursive(dst, tnode->block, vars);
              f = false;
            }
          }
        } else {
          while (tnode->type != TNT_ENUM_LAST && (tnode = tnode->next) != NULL && (tnode->type == TNT_ENUM_ITEM || tnode->type == TNT_ENUM_LAST)) { tnode = tnode->next; }
        }
        break;
      }
      case TNT_INCLUDE: 
      case TNT_INCLUDE_UNLESS: {
	      UStr tn(tnode->block->value, tnode->block->value+tnode->block->value_len);
        tmpl_node_t *block = root->get_template_compiled(tn.str());
        SV** tmp = NULL;
        bool orf = true;
        if (tnode->block->value_len > 0) {
          tmp = template_store::get_variable(tnode->value, tnode->value_len, &vars);
          orf = template_store::chk_variable(tnode->value, tnode->value_len, tmp);
          orf = (tnode->type==TNT_INCLUDE_UNLESS) ? !orf : orf;
	      }
	      if (orf) {
          if (tmp==NULL || SvROK(*tmp) && SvTYPE(SvRV(*tmp)) == SVt_PVHV) {
            if (tmp != NULL) vars.push((HV*) SvRV(*tmp));
            template_store::gen_template_recursive(dst, block, vars);
            if (tmp != NULL) vars.pop();
          }
        }
        break;
      }
      default: {
        return;
      }
    }
    if (tnode == NULL) break;
    tnode = tnode->next;
  }
}

