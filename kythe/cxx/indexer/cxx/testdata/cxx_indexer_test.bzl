def _empty_unless(condition, value):
  if condition: return value
  return "''"

def cxx_indexer_test(name, srcs, deps=[], tags=[], size="small",
                     std="c++1y", ignore_dups=False,
                     ignore_unimplemented=False,
                     index_template_instantiations=True,
                     expect_fail_index=False,
                     expect_fail_verify=False,
                     bundled=False,
                     goal_prefix="//-"):
  if len(srcs) != 1:
    fail("A single source file is required.", "srcs")
  dups = _empty_unless(ignore_dups, "--ignore_dups=true")
  unimplemented = "--ignore_unimplemented="
  if ignore_unimplemented:
    unimplemented += "true"
  else:
    unimplemented += "false"
  expect_fail = "''"
  if expect_fail_index:
    if expect_fail_verify:
      fail("It's not useful to test if a failed index will verify")
    expect_fail = "expectfailindex"
  elif expect_fail_verify:
    expect_fail = "expectfailverify"
  templates = _empty_unless(not index_template_instantiations,
                            "--index_template_instantiations=false")
  goal_prefix_flag = "--goal_prefix=\"" + goal_prefix + "\""
  if bundled:
    native.sh_test(
        name = name,
        srcs = ["//kythe/cxx/indexer/cxx/testdata:bundle_case.sh"],
        data = srcs + deps + [
            "//kythe/cxx/indexer/cxx:indexer",
            "//kythe/cxx/indexer/cxx/testdata:test_vnames.json",
            "//kythe/cxx/indexer/cxx/testdata:handle_results.sh",
            "//kythe/cxx/extractor:cxx_extractor",
            "//kythe/cxx/verifier",
        ],
        args = ["$(location %s)" % srcs[0], std, unimplemented, templates, dups,
                goal_prefix_flag, expect_fail],
        tags = tags,
        size = size,
    )
  else:
    native.sh_test(
        name = name,
        srcs = ["//kythe/cxx/indexer/cxx/testdata:one_case.sh"],
        data = srcs + deps + [
            "//kythe/cxx/indexer/cxx:indexer",
            "//kythe/cxx/indexer/cxx/testdata:handle_results.sh",
            "//kythe/cxx/verifier",
        ],
        args = ["$(location %s)" % srcs[0], std, unimplemented, templates, dups,
                goal_prefix_flag, expect_fail],
        tags = tags,
        size = size,
    )
