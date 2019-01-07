"""This module builds tests for vname rules."""

load("//tools:build_rules/testing.bzl", "shell_tool_test")

# Test a JSON file of rewrite rules against a JSON file of tests.
def test_vname_rules(name, rules, tests):
    shell_tool_test(
        name = name,
        tools = {
            "RULES": rules,
            "TESTS": tests,
            "TOOL": "//kythe/go/util/vnameutil:test_vname_rules",
        },
        script = ['"$$TOOL" --rules="$$RULES" --tests="$$TESTS"'],
    )
