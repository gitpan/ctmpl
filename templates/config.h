#define CTMPL_VERSION "2.0"

#ifndef PAGE_SIZE
  #define PAGE_SIZE 4096
#endif

#define CGI_RANDOM_PARAMETER "random"   //!< Name random parameter
#define CGI_RANDOM_PARAMETER_LEN 6      //!< Length of random parameter
#define MIN_STRING_REALLOC PAGE_SIZE    //!< The minimum number of bytes for realloc in strings.cpp will bring to this magnitude
#define STRING_REALLOC_THRESHOLD (MIN_STRING_REALLOC - 1024) //!< The number of bytes if not quite up to the end of the buffer added another buffer immediately, to avoid unnecessary reallocs.

#define DEFAULT_STRING_LENGTH 1024    //!< By default, creating a buffer for the strings of that size.
#define MAX_DIGIT_LENGTH 48           //!< The length of the buffer to a text record numbers.
#define MAX_SNPRINTF_LENGTH 2048      //!< The length of buffer under sprintf for strings

#define MAX_VARS_DEPTH 100            //!< The maximum number of nesting variables. This parameter determines the number of nested structures.
#define TS_CHECK_UPDATE 5             //!< Time in sec when check update templates
