# Lint as: python3
"""Generate zip_err_str source from zip.h"""

import io
import re

TEMPLATE = r"""/*
  This file was generated automatically by genziperrs.py
  from zip.h; make changes there.
*/

#include "zipint.h"

const char * const _zip_err_str[] = {{
{zip_err_str}}};

const int _zip_nerr_str = sizeof(_zip_err_str)/sizeof(_zip_err_str[0]);

#define N ZIP_ET_NONE
#define S ZIP_ET_SYS
#define Z ZIP_ET_ZLIB

const int _zip_err_type[] = {{
{zip_err_type}}};
"""

DEFINE_PATTERN = re.compile(
    r"#define ZIP_ER_([A-Z_]+) ([0-9]+)[ \t]+/([-*0-9a-zA-Z ']*)/")
ERROR_PATTERN = re.compile(r"([NSZ]+) ([-0-9a-zA-Z ']*)")

def main(argv):
  if len(argv) != 3:
    sys.stderr.write("usage: genziperrs.py <input> <output>\n")
    return 1
  zip_err_str = io.StringIO()
  zip_err_type = io.StringIO()
  with open(argv[1]) as input:
    for line in input:
      match = DEFINE_PATTERN.match(line)
      if match is None:
        continue
      match = ERROR_PATTERN.search(match.group(3))
      if match is None:
        continue
      zip_err_type.write("    " + match.group(1) + ",\n")
      zip_err_str.write("    \"" + match.group(2).strip() + "\",\n")
  with open(argv[2], "w") as output:
    output.write(
        TEMPLATE.format(
            zip_err_str=zip_err_str.getvalue(),
            zip_err_type=zip_err_type.getvalue()))


if __name__ == "__main__":
  import sys
  sys.exit(main(sys.argv))
