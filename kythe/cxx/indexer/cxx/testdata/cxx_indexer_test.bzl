def _empty_unless(condition, value):
  if condition: return value
  return "''"

def cxx_indexer_test(name, srcs, deps=[], tags=[], size="small",
                     std="c++1y", ignore_dups=False,
                     ignore_unimplemented=False,
                     index_template_instantiations=True,
                     goal_prefix="//-"):
  if len(srcs) != 1:
    fail("A single source file is required.", "srcs")
  dups = _empty_unless(ignore_dups, "--ignore_dups=true")
  unimplemented = "--ignore_unimplemented="
  if ignore_unimplemented:
    unimplemented += "true"
  else:
    unimplemented += "false"
  templates = _empty_unless(not index_template_instantiations,
                            "--index_template_instantiations=false")
  goal_prefix_flag = "--goal_prefix=\"" + goal_prefix + "\""
  native.sh_test(
      name = name,
      srcs = ["//kythe/cxx/indexer/cxx/testdata:run_case.sh"],
      deps = ["//kythe/cxx/indexer/cxx/testdata:one_case"],
      data = srcs + deps + [
          "//kythe/cxx/indexer/cxx:indexer",
          "//kythe/cxx/verifier",
      ],
      args = ["$(location %s)" % srcs[0], std, dups, unimplemented, templates,
              goal_prefix_flag],
      tags = tags,
      size = size,
  )
