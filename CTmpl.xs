#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#include <templates/templates.h>

MODULE = CTmpl		PACKAGE = CTmpl		

PROTOTYPES: DISABLE

void
CTmpl::DESTROY()

CTmpl*
CTmpl::new()

SV *
CTmpl::version()
OUTPUT:
  RETVAL
  
SV *
CTmpl::add_templates(IN HV *A)
OUTPUT:
  RETVAL

SV *
CTmpl::gen_template(IN SV *A, IN HV *B)
OUTPUT:
  RETVAL

SV *
CTmpl::get_compiled_struct(IN SV *A)
OUTPUT:
  RETVAL

