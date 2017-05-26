# Test a JSON file of rewrite rules against a JSON file of tests.
def test_vname_rules(name, rules, tests, size="small"):
  tool = "//kythe/go/util/vnameutil:test_vname_rules"
  script = name+"_script"
  native.genrule(
      name = script,
      outs = [name+".sh"],
      cmd  = "echo set -x ';' kythe/go/util/vnameutil/test_vname_rules " + \
             "--rules='$$1' --tests='$$2' > '$@'",
      executable = True,
      output_to_bindir = True,
      testonly = True,
  )
  native.sh_test(
      name = name,
      size = size,
      srcs = [script],
      data = [":"+rules, ":"+tests, tool],
      args = ["$(location :%s)" % rules, "$(location :%s)" % tests],
  )
