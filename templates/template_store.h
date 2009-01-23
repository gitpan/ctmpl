#ifndef CTMPL_TEMPLATE_STORE_H
#define CTMPL_TEMPLATE_STORE_H

#include <EXTERN.h>
#include <perl.h>
#include <XSUB.h>

#include "sys/timex.h"

#include "config.h"
#include "strings.h"

class CTmpl;
#ifndef byte
typedef unsigned char byte;
#endif
#ifndef ulong
typedef unsigned long ulong;
#endif

enum {
  TNT_TEXT = 0,
  TNT_SIMPLE,
  TNT_INCLUDE,
  TNT_INCLUDE_UNLESS,
  TNT_IF,
  TNT_UNLESS,
  TNT_ELSE,
  TNT_LOOP,
  TNT_ENUM,
  TNT_ENUM_ITEM,
  TNT_ENUM_LAST
};

enum {
  TV_NORMAL = 0,
  TV_FIRST,
  TV_LAST,
  TV_ODD,
  TV_EVEN
};

typedef struct tmpl_node_t {
  char *pre_src;
  ulong pre_src_len;
  
  long  enum_records;

  byte  type;
  
  byte  value_type;
  char  *value;
  ulong value_len;
  
  tmpl_node_t *block;
  
  tmpl_node_t *next;
  
} tmpl_node_t;

class list_vars {
public:
  HV* vars[MAX_VARS_DEPTH];
  int cur;
  
  SV* special_vars[TV_EVEN+1];
  
  void push(HV *hv) { vars[cur++] = hv; }
  
  HV* pop() {
    memset(special_vars, 0, sizeof(special_vars));
    return (cur>0) ? vars[cur--] : NULL;
  }

  list_vars() {
    memset(vars, 0, MAX_VARS_DEPTH);
    cur = 0;
    memset(special_vars, 0, sizeof(special_vars));
  }
  list_vars(list_vars *old) {
    memcpy(vars, old->vars, MAX_VARS_DEPTH);
    cur = old->cur;
    memcpy(special_vars, old->special_vars, sizeof(special_vars));
  }
  ~list_vars() {
  }
  
};

class template_store {

  CTmpl *root;

  char*   fname;
  time_t  mtm;
  time_t  last_check;
  char*   body;
  long    body_size;
  char*   max;
  
  tmpl_node_t *compiled;

  void compile_template();
  UStr *errors;
  
  char *comp_helper;
  UStr *buffer;
  UStr *comp_struct;

  void comp_error(const char *, const char *, ...);
  const char* comp_static(const char *, tmpl_node_t**&);
  char* comp_simple(const char *, tmpl_node_t**&);
  char* comp_ifelse(const char *, tmpl_node_t **&);
  char* comp_loop(const char *, tmpl_node_t **&);
  char* comp_enum(const char *, tmpl_node_t **&);
  char* comp_include(const char *, tmpl_node_t **&);

  char* comp_genblock(const char *, const char *, tmpl_node_t**);

  void clear();
  void free_compiled(tmpl_node_t *);

  void r_get_compiled_struct(UStr *, long, tmpl_node_t *);
  
  static void  find_line_pos(const char *, const char *, long &, long &);
  static SV**  get_variable(char *, I32, list_vars *);
  static bool  chk_variable(char *, I32, SV **);
  
  static byte  check_value_type(const char *, ulong size);


public:

  static char* load(const char *, time_t &, long &);

  char*   filename() const { return fname; }
  time_t  modified() const { return mtm; }
  char*   get_body() const { return body; }
  long    get_body_size() const { return body_size; }
  char*   get_max() const { return max; }
  
  tmpl_node_t* get_compiled() const { return compiled; }
  
  UStr*   get_compiled_struct();
  char*   get_errors() const { return (errors!=NULL)?errors->str():NULL; }
  
  UStr*   gen_template(list_vars &);
  void    gen_template_recursive(UStr *, tmpl_node_t *, list_vars &);
  
  template_store(char *, long, CTmpl *);
  ~template_store();
};

#endif
