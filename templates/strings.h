#ifndef strings_h
#define strings_h

#include <string>

class Strings {

  char *buffer;
  long maxsize;
  bool nf;
  int  realloc_modifier;
  long original_size;
  long strl;

  Strings& append(long);
  Strings& append(float);
  Strings& append(ulong);
  Strings& append(const char *);
  Strings& append(const char *, long);
  
  Strings& assign(const char *);
  Strings& assign(long);
  Strings& assign(float);
  Strings& assign(ulong);
  
  Strings& append_before(const char *);
  Strings& append_before(const char *, long);

  Strings& insert_at(long, long, const char *);

  long Unslash(const char *, const char *, long);
    
  void CreateString(const char *, long, long, bool, bool);
  
public:

  void set_nf() { nf = true; }
  char* str() const { return buffer; }
  char* nf_str() { nf = true; return buffer; }
  long max_size() const { return maxsize; }
  
  void cat(const char *src) { append(src); }
  void cat(long digit) { append(digit); }
  void ncat(const char *src, long sz) { append(src, sz); }
  void snprintf(const char *format, ...);
  void catsnprintf(const char *format, ...);

  void insert(long offset, long size, const char *src) { insert_at(offset, size, src); }
  
  long length() const { return (strl>=0)?strl:strlen(buffer); }

  void shrink(long);
  void clear();
  void unslash() { strl = Unslash(buffer, buffer, strl); }
  int replace(char *, char *);
  int slash_js();
  void trim_spaces();
  char* str_trimmed() { trim_spaces(); return buffer; }

  Strings& operator+=(const char* src) { return append(src); }
  Strings& operator+=(int digit) { return append((long) digit); }
  Strings& operator+=(int16_t digit) { return append((long) digit); }
  Strings& operator+=(long digit) { return append(digit); }
  Strings& operator+=(ulong digit) { return append(digit); }
  Strings& operator+=(float digit) { return append(digit); }

  Strings& operator-=(const char* src) { return append_before(src); }

  char& operator[](long pos) { if (pos<maxsize && pos>=0) { return *(buffer+pos); } else { return *(buffer+maxsize-1); } }
  Strings& operator=(const char* src) { return assign(src); }
  Strings& operator=(int digit) { return assign((long) digit); }
  Strings& operator=(int16_t digit) { return assign((long) digit); }
  Strings& operator=(long digit) { return assign(digit); }
  Strings& operator=(ulong digit) { return assign(digit); }
  Strings& operator=(float digit) { return assign(digit); }

  Strings();
  Strings(long);
  Strings(const char *);
  Strings(long, const char *);
  Strings(long, const char *, int);
  Strings(const char *, int);
  Strings(const char *, long);
  Strings(const char *, const char *);
  Strings(const char *, const char *, bool);
  Strings(const char *, bool);
  Strings(const char *, bool, bool);
  
  ~Strings();
  
};

typedef class Strings UStr;
#endif
