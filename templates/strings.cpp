
#include "strings.h"
#include "config.h"

#include <cstdarg>

Strings::Strings() {

  realloc_modifier = 0;
  maxsize = MIN_STRING_REALLOC;
  buffer = (char *) malloc(maxsize);
  nf = false;
  original_size = 0;
  strl = 0;
  clear();
}

Strings::Strings(long msz) {
  if (msz <= 0) msz = 1;
  realloc_modifier = 0;
  maxsize = msz;
  buffer = (char *) malloc(maxsize);
  nf = false;
  original_size = 0;
  strl = 0;
  clear();
}

Strings::Strings(const char *src) {

  long size = strlen(src), newsize = size+1;
  if (newsize < MIN_STRING_REALLOC) {
    newsize = MIN_STRING_REALLOC;
  } else if (newsize > MIN_STRING_REALLOC) {
    long ntr = newsize / MIN_STRING_REALLOC;
    newsize = (ntr+((newsize - (ntr * MIN_STRING_REALLOC) > STRING_REALLOC_THRESHOLD)?2:1)) * MIN_STRING_REALLOC;
  }
  Strings::CreateString(src, size, newsize, false, false);
}

Strings::Strings(long msz, const char *src, int sz) {

  realloc_modifier = 0;
  maxsize = msz;
  buffer = (char *) malloc(maxsize);
  nf = false;
  clear();
  if (sz > msz) sz = msz-1;
  memcpy(buffer, src, sz);
  original_size = sz;
  strl = sz;
}

Strings::Strings(long msz, const char *src) {

  int sz = strlen(src);
  realloc_modifier = 0;
  maxsize = msz;
  buffer = (char *) malloc(maxsize);
  nf = false;
  clear();
  if (sz > msz) sz = msz-1;
  memcpy(buffer, src, sz);
  original_size = sz;
  strl = sz;
}

Strings::Strings(const char *src, int sz) {
  Strings::CreateString(src, strlen(src), (long)sz, false, false);

}

Strings::Strings(const char *src, long sz) {

  Strings::CreateString(src, strlen(src), sz, false, false);
}

Strings::Strings(const char *begin, const char *end) {

  Strings::CreateString(begin, (long)(end-begin), (long)(end-begin)+1, false, false);
}

Strings::Strings(const char *begin, const char *end, bool slash) {

  long newsize = (long)(end-begin)+1;
  if (newsize < MIN_STRING_REALLOC) {
    newsize = MIN_STRING_REALLOC;
  } else if (newsize > MIN_STRING_REALLOC) {
    long ntr = newsize / MIN_STRING_REALLOC;
    newsize = (ntr+((newsize - (ntr * MIN_STRING_REALLOC) > STRING_REALLOC_THRESHOLD)?2:1)) * MIN_STRING_REALLOC;
  }
  Strings::CreateString(begin, (long)(end-begin), newsize, false, false);
  //slash_js();
}


Strings::Strings(const char *src, bool nocopy) {

  long size = strlen(src);
  long newsize = size+1;
  if (newsize < MIN_STRING_REALLOC) {
    newsize = MIN_STRING_REALLOC;
  } else if (newsize > MIN_STRING_REALLOC) {
    long ntr = newsize / MIN_STRING_REALLOC;
    newsize = (ntr+((newsize - (ntr * MIN_STRING_REALLOC) > STRING_REALLOC_THRESHOLD)?2:1)) * MIN_STRING_REALLOC;
  }
  Strings::CreateString(src, size, newsize, nocopy, false);
}

Strings::Strings(const char *src, bool nocopy, bool nofree) {

  long size = strlen(src);
  long newsize = size+1;
  if (newsize < MIN_STRING_REALLOC) {
    newsize = MIN_STRING_REALLOC;
  } else if (newsize > MIN_STRING_REALLOC) {
    long ntr = newsize / MIN_STRING_REALLOC;
    newsize = (ntr+((newsize - (ntr * MIN_STRING_REALLOC) > STRING_REALLOC_THRESHOLD)?2:1)) * MIN_STRING_REALLOC;
  }
  Strings::CreateString(src, size, newsize, nocopy, nofree);
}

void Strings::CreateString(const char *src, long sz, long msz, bool nocopy, bool nofree) {
  
  original_size = 0;
  realloc_modifier = 0;
  if (msz <= 0) msz = 1;
  maxsize = msz;
  if (sz > msz) sz = msz-1;
  nf = nofree;
  strl = sz;
  if (nocopy) {
    buffer = (char *) src;
    maxsize = 0;
  } else {
    original_size = sz;
    buffer = (char *) malloc(maxsize);
    if (sz > 0)
      memcpy(buffer, src, sz);
    *(buffer+sz) = 0;
    nf = false;
  }
}

Strings::~Strings() {

  if (buffer != NULL) {
    if (!nf) {
      free(buffer);
    }
  }
}


Strings& Strings::append(long digit) {
  
  char str[MAX_DIGIT_LENGTH];
  std::snprintf(str, MAX_DIGIT_LENGTH, "%ld", digit);
  
  return append(str, strlen(str));
}

Strings& Strings::append(ulong digit) {
  
  char str[MAX_DIGIT_LENGTH];
  std::snprintf(str, MAX_DIGIT_LENGTH, "%lu", digit);
  
  return append(str, strlen(str));
}

Strings& Strings::append(float digit) {
  
  char str[MAX_DIGIT_LENGTH];
  std::snprintf(str, MAX_DIGIT_LENGTH, "%.2f", digit);
  
  return append(str, strlen(str));
}

Strings& Strings::append(const char* src) {
  
  return append(src, (src)?strlen(src):0);
}

Strings& Strings::append(const char* src, long sz) {

  if (sz <= 0 || src == NULL)
    return *this;
  
  int size = strl;
  long newsize = size+sz+1;
  
  if (newsize > maxsize) {
    long ntr = newsize / MIN_STRING_REALLOC;
    newsize = (realloc_modifier + ntr+((newsize - ((realloc_modifier + ntr) * MIN_STRING_REALLOC) > STRING_REALLOC_THRESHOLD)?2:1)) * MIN_STRING_REALLOC;
    realloc_modifier++;
    char* tmp = (char*) realloc(buffer, newsize);
    if (tmp == NULL) {
      return *this;
    }
    buffer = tmp;
    maxsize = newsize;
  }
  
  memcpy(buffer+size, src, sz);
  *(buffer+size+sz) = 0;

  strl = size+sz;

  return *this;
}

Strings& Strings::append_before(const char* src) {
  
  return append_before(src, strlen(src));
}

Strings& Strings::append_before(const char* src, long sz) {

  if (sz <= 0 || src == NULL)
    return *this;
  
  
  long newsize = sz+strl+1;
  
  if (newsize > maxsize) {
    long ntr = newsize / MIN_STRING_REALLOC;
    newsize = (realloc_modifier + ntr+((newsize - ((realloc_modifier + ntr) * MIN_STRING_REALLOC) > STRING_REALLOC_THRESHOLD)?2:1)) * MIN_STRING_REALLOC;
    
    realloc_modifier++;
    char* tmp = (char*) realloc(buffer, newsize);
    if (tmp == NULL) {
      return *this;
    }
    buffer = tmp;
    maxsize = newsize;
  }
  memmove(buffer+sz, buffer, maxsize-sz);
  memcpy(buffer, src, sz);
  strl = strl + sz;
  
  return *this;
}

Strings& Strings::assign(long digit) {
  
  char str[MAX_DIGIT_LENGTH];
  std::snprintf(str, MAX_DIGIT_LENGTH, "%ld", digit);
  
  return assign(str);
}

Strings& Strings::assign(ulong digit) {
  
  char str[MAX_DIGIT_LENGTH];
  std::snprintf(str, MAX_DIGIT_LENGTH, "%lu", digit);
  
  return assign(str);
}

Strings& Strings::assign(float digit) {
  
  char str[MAX_DIGIT_LENGTH];
  std::snprintf(str, MAX_DIGIT_LENGTH, "%.2f", digit);
  
  return assign(str);
}

Strings& Strings::assign(const char* src) {

  long sz = strlen(src);
  long newsize = sz+1;
  strl = sz;
  if (newsize > maxsize) {
    long ntr = newsize / MIN_STRING_REALLOC;
    newsize = (realloc_modifier + ntr+((newsize - ((realloc_modifier + ntr) * MIN_STRING_REALLOC) > STRING_REALLOC_THRESHOLD)?2:1)) * MIN_STRING_REALLOC;
    realloc_modifier++;
    
    char* tmp = (char*) realloc(buffer, newsize);
    if (tmp == NULL) {
      return *this;
    }
    buffer = tmp;
    maxsize = newsize;
  }
  memcpy(buffer, src, sz);
  *(buffer+sz) = 0;

  return *this;
}

void Strings::snprintf(const char *format, ...) {

	va_list	ap;
	va_start(ap, format);
    
  char buf[MAX_SNPRINTF_LENGTH+1] = { 0 };
	std::vsnprintf(buf, MAX_SNPRINTF_LENGTH, format, ap);
  Strings::assign(buf);
  
  va_end(ap);
}

void Strings::catsnprintf(const char *format, ...) {

	va_list	ap;
	va_start(ap, format);
  
  char buf[MAX_SNPRINTF_LENGTH+1] = { 0 };
	std::vsnprintf(buf, MAX_SNPRINTF_LENGTH, format, ap);
  Strings::append(buf);
  
  va_end(ap);
}

long Strings::Unslash(const char *str, const char *bf, long sz) {
  
  char *cur, *src = (char*)str, *dst = (char*)bf;
  
  while(src < str+sz) {
    cur = strchr(src, '\\');
    if (cur != NULL && cur < str+sz) {
      int len = cur-src;
      strncpy(dst, src, len);
      dst+=len;
    
      cur++;
      if (cur >= str+sz)
        break;
      switch(*cur) {
        case 'n': *dst++ = '\n'; break;
        case 't': *dst++ = '\t'; break;
        default: *dst++ = *cur; break;
      }
      src = ++cur;
    } else {
      int len = (str+sz)-src;
      strncpy(dst, src, len);
      dst+=len;
      break;
    }
  }
  *dst = 0;
  return dst - str;
}

void Strings::shrink(long newsize) {
  
  if (newsize >= maxsize)
    return;

  if (newsize < 0) {
    if (original_size > 0)
      newsize = original_size;
    else
      return;
  }

  strl = -1;
  realloc_modifier /= 2;
  *(buffer+newsize-1) = 0;
  
  if (newsize < MIN_STRING_REALLOC)
    newsize = MIN_STRING_REALLOC;
  
  if (maxsize <= newsize)
    return;

  char* tmp = (char*) realloc(buffer, newsize);
  if (tmp == NULL)
    return;

  buffer = tmp;
  maxsize = newsize;
  *(buffer+newsize-1) = 0;
  return;
}

void Strings::clear() {
  memset(buffer, 0, maxsize);
  strl = 0;
}

Strings& Strings::insert_at(long offset, long sz, const char *gsrc) {
  
  if (sz <= 0 || gsrc == NULL)
    return *this;
    
  char *src = (char*) gsrc;
  long newsize = offset + sz + 1;
  bool setzero = false;
  if (newsize > maxsize) {
    long ntr = newsize / MIN_STRING_REALLOC;
    newsize = (ntr+((newsize - (ntr * MIN_STRING_REALLOC) > STRING_REALLOC_THRESHOLD)?2:1)) * MIN_STRING_REALLOC;
    char* tmp = (char*) realloc(buffer, newsize);
    if (tmp == NULL)
      return *this;
    if (buffer >= src && src < (buffer+maxsize))
      src = (src-buffer)+tmp;
    buffer = tmp;
    memset(buffer+maxsize, 0, newsize-maxsize);
    maxsize = newsize;
    setzero = true;
  }
  memmove(buffer+offset, src, sz);
  if (setzero) {
    *(buffer+offset+sz) = 0;
  }
  
  strl = -1;
  
  return *this;

}

int Strings::replace(char *substr, char *replace) {
  int sub_size = strlen(substr);
  int rep_size = strlen(replace);
  if (sub_size <= 0)
    return 0;
  
  char *cur, *old_buffer, *buf = buffer;
  char *smax = buffer + length();
  int count = 0;
  while ((cur = strstr(buf, substr)) != NULL) {
    old_buffer = buffer;
    int length = strl;
    insert_at(cur-buffer+rep_size, (smax-cur)+sub_size, cur+sub_size);
    if (old_buffer != buffer) cur = (cur-old_buffer) + buffer;
    if (rep_size > 0) {
      old_buffer = buffer;
      insert_at(cur-buffer, rep_size, replace);
      smax = (smax-old_buffer) + buffer + 1;
      if (old_buffer != buffer) cur = (cur-old_buffer) + buffer;
      if (cur-buffer + rep_size > length)
        *(cur+rep_size) = 0;
    }
    buf = cur+rep_size;
    if (rep_size - sub_size < 0)
      *(buffer+rep_size-sub_size+length) = 0;
    count++;
  }
  
  return count;
}

int Strings::slash_js() {
  char *cur, *old_buffer, *buf = buffer;
  char *smax = buffer + length();
  int count = 0;
  while ((cur = strpbrk(buf, "'\"\\<>&\n")) != NULL) {
    old_buffer = buffer;
    insert_at(cur-buffer+1, smax-cur, cur);
    cur = (cur-old_buffer) + buffer;
    smax = (smax-old_buffer) + buffer + 1;
    if (*cur == '\n') {
      *cur++ = '\\';
      *cur = 'n';
    } else if (*cur == '\r') {
      *cur++ = '\\';
      *cur = 'r';
    } else 
      *cur++ = '\\';
    buf = cur+1;
    count++;
  }
  return count;
}

void Strings::trim_spaces() {
  char *buf = buffer, *dst = buf, *last = NULL;
  unsigned char a;
  bool lead = true;
  while( (a = *buf) != 0 ) {
    if (lead && (a == ' ' || a == '\n' || a == '\t' || a == '\r')) {
      buf++;
    } else {
      lead = false;
      *dst++ = *buf++;
      if (a != ' '  && a != '\n' && a != '\t' && a != '\r')
        last = dst;
    }
  }
  if (last != NULL)
    *last = 0;
  strl = last - buffer;

}


