#ifndef KYTHE_CXX_EXTRACTOR_CLANG_BUILTIN_HEADERS_H_
#define KYTHE_CXX_EXTRACTOR_CLANG_BUILTIN_HEADERS_H_

#include <stddef.h>

struct FileToc {
  const char *name;
  const char *data;
};

extern const struct FileToc* builtin_headers_create();

#endif
