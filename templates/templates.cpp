#include "templates.h"

CTmpl::CTmpl() {
#ifdef FAST
  Templates.set_empty_key(NULL);
  Templates.set_deleted_key("__deleted__");
#endif
};

CTmpl::~CTmpl() {
  for (hash_map_t::iterator i = Templates.begin(); i != Templates.end(); ++i) {
    free((void*)i->first);
    delete i->second;
  }
  Templates.clear();
};

SV* CTmpl::version() {
  return  newSVpvf("CTmpl version %s", CTMPL_VERSION);
}

SV* CTmpl::add_templates(HV *Tmpls) {
  if (SvTYPE(Tmpls) != SVt_PVHV)
    return newSVpvf("ERROR: only reference to hash accepted! (debug: %ld)", SvTYPE(Tmpls));

  SV      *value;
  char    *key, *val;
  I32     klen;
  STRLEN  vlen;
  UStr err;
  for (I32 i = hv_iterinit(Tmpls); i > 0; --i) {
    value = hv_iternextsv(Tmpls, &key, &klen);
    val = SvPV(value, vlen); 
    char *tkey = (char*) malloc(klen+1);
    memcpy(tkey, key, klen);
    *(tkey+klen) = 0;
    template_store* ts = new template_store(val, vlen, this);
    if (Templates.count(tkey) > 0) delete Templates[tkey];
    Templates[tkey] = ts;
    
    err.cat(ts->get_errors());
  }
  if (err.length() > 0) {
    SV *sv = newSV(err.length()+1);
    sv_setpvn(sv, err.str(), err.length());
    return sv;
  }
  return &PL_sv_undef;
}

SV* CTmpl::gen_template(SV *tmpl_name, HV *prms) {
  
  STRLEN  klen;
  char   *key = SvPV(tmpl_name, klen);
  if (Templates.count(key) <= 0) return &PL_sv_undef;

  list_vars vars;
  vars.push(prms);
  UStr *str = Templates[key]->gen_template(vars);
  
  SV *sv = newSV(str->length()+1);
  sv_setpvn(sv, str->str(), str->length());

  return sv;
}

SV* CTmpl::get_compiled_struct(SV *tmpl_name) {
  STRLEN  klen;
  char   *key = SvPV(tmpl_name, klen);
  if (Templates.count(key)) {
    UStr *tmp = Templates[key]->get_compiled_struct();
    
    SV *sv = newSV(tmp->length()+1);
    sv_setpvn(sv, tmp->str(), tmp->length());
  
    return sv;
  }
  return &PL_sv_undef;
}

tmpl_node_t* CTmpl::get_template_compiled(char *tname) {

  if (Templates.count(tname)) {
    return Templates[tname]->get_compiled();
  }
  return NULL;
}

