package(default_visibility = ["//kythe:default_visibility"])

load("ts_indexer_test", "ts_indexer_test")

filegroup(
    name = "repo_files",
    srcs = glob(["repo/**/*"]),
)

sh_binary(
    name = "run_case",
    srcs = ["run_case.sh"],
    data = [
        "//kythe/ts:ts_indexer",
        "//kythe/go/platform/tools:entrystream",
        "//tools:node",
        "//kythe/cxx/verifier",
    ],
)

test_suite(
    name = "basic_tests",
    tags = ["basic"],
)

ts_indexer_test(
    name = "test_test",
    srcs = ["basic/test_test.js"],
    tags = ["basic"],
)

ts_indexer_test(
    name = "var_ref",
    srcs = ["basic/var_ref.js"],
    tags = ["basic"],
)

ts_indexer_test(
    name = "utf8_comments",
    srcs = ["basic/utf8_comments.js"],
    tags = ["basic"],
)

ts_indexer_test(
    name = "utf8_comments_surrogate_pair",
    srcs = ["basic/utf8_comments_surrogate_pair.js"],
    tags = ["basic"],
)

test_suite(
    name = "commonjs_tests",
    tags = ["commonjs"],
)

ts_indexer_test(
    name = "require_object",
    needs_node = True,
    srcs = [
      "commonjs/export_object.js",
      "commonjs/require_object.js",
    ],
    tags = ["commonjs"],
)

ts_indexer_test(
    name = "require_npm",
    needs_node = True,
    srcs = [
      "commonjs/require_npm.js",
      "commonjs/node_modules/mod_from_npm/package.json",
      "commonjs/node_modules/mod_from_npm/mod_from_npm_main.js",
    ],
    tags = ["commonjs"],
)

test_suite(
    name = "repo_tests",
    tags = ["repo"],
)

ts_indexer_test(
    name = "repo_index",
    needs_node = True,
    srcs = [
      "repo/index.js",
    ],
    deps = [":repo_files"],
    tags = ["repo"],
)
