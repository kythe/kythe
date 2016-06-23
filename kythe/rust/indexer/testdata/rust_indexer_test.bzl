def rust_indexer_test(name, srcs, deps=[], tags=[], size="small",
                      ignore_dups=False,
                      crate_type="lib",
                      expect_fail_index=False,
                      expect_fail_verify=False,
                      goal_prefix="//-"):
  args = ["$(location %s)" % srcs[0], crate_type, "--verifier",
          "--goal_prefix=\"" + goal_prefix + "\""]
  for src in srcs:
    args += ["--verifier", "$(location %s)" % src]
  if ignore_dups:
    args += ["--verifier", "--ignore_dups=true"]
  if expect_fail_index:
    if expect_fail_verify:
      fail("It's not useful to test if a failed index will verify")
    args += ["--expected", "expectfailindex"]
  elif expect_fail_verify:
    args += ["--expected", "expectfailverify"]
  native.sh_test(
      name = name,
      srcs = ["//kythe/rust/indexer/testdata:one_case.sh"],
      data = srcs + deps + [
          "//kythe/rust/indexer:libindexer",
          "//kythe/cxx/verifier",
          "//kythe/go/platform/tools:entrystream",
          "//kythe/rust/indexer/testdata:handle_results.sh",
          "//kythe/rust/indexer/testdata:parse_args.sh",
      ],
      args = args,
      tags = tags + ["local", "manual"],
      size = size,
  )
