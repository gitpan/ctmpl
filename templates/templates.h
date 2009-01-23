#ifndef CTMPL_TEMPLATES_H
#define CTMPL_TEMPLATES_H

#include "template_store.h"

#include <sys/user.h>

#ifdef FAST
  #undef do_open
  #undef do_close

  #include "google/dense_hash_map"
  struct eqstr {
    bool operator()(const char* s1, const char* s2) const {
      return (s1 == s2) || (s1 && s2 && strcmp(s1, s2) == 0);
    }
  };
  typedef google::dense_hash_map<const char*, template_store *, __gnu_cxx::hash<const char*>, eqstr> hash_map_t;
#else
  #undef do_open
  #undef do_close

  #include <hash_map.h>
  struct eqstr {
    bool operator()(const char* s1, const char* s2) const {
      return (s1 == s2) || (s1 && s2 && strcmp(s1, s2) == 0);
    }
  };
  typedef hash_map<const char*, template_store *, __gnu_cxx::hash<const char*>, eqstr> hash_map_t;
#endif

class CTmpl {

  hash_map_t Templates;

public:

  SV* version();
  SV* add_templates(HV *);
  SV* gen_template(SV *, HV *);
  
  SV* get_compiled_struct(SV *);
  
  tmpl_node_t* get_template_compiled(char *);


  CTmpl();
  ~CTmpl();

};
#endif
