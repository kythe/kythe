#include "clang_builtin_headers.h"
#include "../include/cxx_extractor_resources.inc"

const struct FileToc *builtin_headers_create() {
  return kClangCompilerIncludes;
}
