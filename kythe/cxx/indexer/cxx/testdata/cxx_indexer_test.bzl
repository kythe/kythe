def cxx_indexer_test(name, srcs, deps=[], tags=[], size="small",
                     std="c++1y", ignore_dups=False,
                     ignore_unimplemented=False,
                     index_template_instantiations=True,
                     expect_fail_index=False,
                     expect_fail_verify=False,
                     bundled=False,
                     experimental_drop_instantiation_independent_data=False,
                     goal_prefix="//-"):
  if len(srcs) != 1:
    fail("A single source file is required.", "srcs")
  args = ["$(location %s)" % srcs[0], std, "--verifier",
          "--goal_prefix=\"" + goal_prefix + "\""]
  if ignore_dups:
    args += ["--verifier", "--ignore_dups=true"]
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
    native.sh_test(
        name = name,
        srcs = ["//kythe/cxx/indexer/cxx/testdata:bundle_case.sh"],
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
    native.sh_test(
        name = name,
        srcs = ["//kythe/cxx/indexer/cxx/testdata:one_case.sh"],
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
