def _empty_unless(condition, value):
  if condition: return value
  return "''"

def cxx_indexer_test(name, srcs, deps=[], tags=[], size="small",
                     std="c++1y", ignore_dups=False,
                     ignore_unimplemented=False,
                     index_template_instantiations=True):
  if len(srcs) != 1:
    fail("A single source file is required.", "srcs")
  dups = _empty_unless(ignore_dups, "--ignore_dups=true")
  unimplemented = _empty_unless(ignore_unimplemented, "--ignore_unimplemented")
  templates = _empty_unless(not index_template_instantiations,
                            "--index_template_instantiations=false")
  native.sh_test(
      name = name,
      srcs = ["//kythe/cxx/indexer/cxx/testdata:run_case.sh"],
      deps = ["//kythe/cxx/indexer/cxx/testdata:one_case"],
      data = srcs + deps + [
          "//kythe/cxx/indexer/cxx:indexer",
          "//kythe/cxx/verifier",
      ],
      args = ["$(location %s)" % srcs[0], std, dups, unimplemented, templates],
      tags = tags,
      size = size,
  )
