# If a test is expected to pass on darwin but not on linux, you can set
# darwin_only to True. This causes the test to always pass on linux and it
# causes the actual test to execute on darwin.
#
# Setting objc to True will also set std to "".
def cxx_indexer_test(name, srcs, deps=[], tags=[], size="small",
                     std="c++1y", ignore_dups=False,
                     ignore_unimplemented=False,
                     fail_on_unimplemented_builtin=True,
                     index_template_instantiations=True,
                     expect_fail_index=False,
                     expect_fail_verify=False,
                     bundled=False,
                     objc=False,
                     experimental_drop_instantiation_independent_data=False,
                     darwin_only=False,
                     convert_marked_source=False,
                     goal_prefix="//-"):
  if len(srcs) != 1:
    fail("A single source file is required.", "srcs")
  args = ["$(location %s)" % srcs[0], "--verifier",
          "--goal_prefix=\"" + goal_prefix + "\""]
  if std != "" and not objc:
    args += ["--clang", "-std=" + std]
  if objc:
    args += ["--clang", "-fblocks"]
  if convert_marked_source:
    args += ["--verifier", "--convert_marked_source=true"]
  if ignore_dups:
    args += ["--verifier", "--ignore_dups=true"]
  if fail_on_unimplemented_builtin:
    args += ["--indexer", "--fail_on_unimplemented_builtin=true"]
  else:
    args += ["--indexer", "--fail_on_unimplemented_builtin=false"]
  if ignore_unimplemented:
    args += ["--indexer", "--ignore_unimplemented=true"]
  else:
    args += ["--indexer", "--ignore_unimplemented=false"]
  if expect_fail_index:
    if expect_fail_verify:
      fail("It's not useful to test if a failed index will verify")
    args += ["--expected", "expectfailindex"]
  elif expect_fail_verify:
    args += ["--expected", "expectfailverify"]
  if index_template_instantiations:
    args += ["--indexer", "--index_template_instantiations=true"]
  else:
    args += ["--indexer", "--index_template_instantiations=false"]
  if experimental_drop_instantiation_independent_data:
    args += ["--indexer",
             "--experimental_drop_instantiation_independent_data=true"]
  else:
    args += ["--indexer",
             "--experimental_drop_instantiation_independent_data=false"]
  if bundled:
    nondarwin_test = []
    if darwin_only:
      nondarwin_test = ["//kythe/cxx/indexer/cxx/testdata:pass_test.sh"]
    else:
      nondarwin_test = ["//kythe/cxx/indexer/cxx/testdata:bundle_case.sh"]
    native.sh_test(
        name = name,
        srcs = select({
            "//:darwin": ["//kythe/cxx/indexer/cxx/testdata:bundle_case.sh"],
            "//conditions:default": nondarwin_test,
        }),
        data = srcs + deps + [
            "//kythe/cxx/indexer/cxx:indexer",
            "//kythe/cxx/indexer/cxx/testdata:test_vnames.json",
            "//kythe/cxx/indexer/cxx/testdata:handle_results.sh",
            "//kythe/cxx/indexer/cxx/testdata:parse_args.sh",
            "//kythe/cxx/extractor:cxx_extractor",
            "//kythe/cxx/verifier",
        ],
        args = args,
        tags = tags,
        size = size,
    )
  else:
    nondarwin_test = []
    if darwin_only:
      nondarwin_test = ["//kythe/cxx/indexer/cxx/testdata:pass_test.sh"]
    else:
      nondarwin_test = ["//kythe/cxx/indexer/cxx/testdata:one_case.sh"]
    native.sh_test(
        name = name,
        srcs = select({
            "//:darwin": ["//kythe/cxx/indexer/cxx/testdata:one_case.sh"],
            "//conditions:default": nondarwin_test,
        }),
        data = srcs + deps + [
            "//kythe/cxx/indexer/cxx:indexer",
            "//kythe/cxx/indexer/cxx/testdata:handle_results.sh",
            "//kythe/cxx/indexer/cxx/testdata:parse_args.sh",
            "//kythe/cxx/verifier",
        ],
        args = args,
        tags = tags,
        size = size,
    )
