def _empty_unless(condition, value):
  if condition: return value
  return "''"

def ts_indexer_test(name, srcs, deps=[], tags=[], size="small",
                    root_dir="kythe/ts/testdata", ignore_dups=False,
                    skip_default_lib=True,
                    needs_node=False,
                    vnames="//kythe/ts/testdata:vnames.json",
                    goal_prefix="//-"):
  dups = _empty_unless(ignore_dups, "--ignore_dups=true")
  skip_default_flag = _empty_unless(skip_default_lib, "--skipDefaultLib")
  goal_prefix_flag = "--goal_prefix=\"" + goal_prefix + "\""
  needs_node_flag = _empty_unless(needs_node, "implicit=kythe/ts/typings/node/node.d.ts")
  root_dir_flag = "--rootDir=" + root_dir
  located_srcs = []
  for src in srcs:
    located_srcs += ["\"$(location %s)\"" % src]
  native.sh_test(
      name = name,
      srcs = ["//kythe/ts/testdata:run_case.sh"],
      data = srcs + deps + [
          vnames,
          "//kythe/cxx/verifier",
          "//kythe/ts/testdata:run_case",
      ],
      args = ["$(location %s)" % vnames, skip_default_flag, root_dir_flag,
          needs_node_flag, "--", dups, goal_prefix_flag, "--"] + located_srcs,
      tags = tags,
      size = size,
  )
