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
        "//external:node",
        "//kythe/cxx/verifier",
        "//kythe/go/platform/tools:entrystream",
        "//kythe/ts:ts_indexer",
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
    name = "function_def",
    srcs = ["basic/function_def.js"],
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

ts_indexer_test(
    name = "local_var_def",
    srcs = ["basic/local_var_def.js"],
    tags = ["basic"],
)

ts_indexer_test(
    name = "var_def",
    srcs = ["basic/var_def.js"],
    tags = ["basic"],
)

ts_indexer_test(
    name = "param_def",
    srcs = ["basic/param_def.js"],
    tags = ["basic"],
)

ts_indexer_test(
    name = "call_ref",
    srcs = ["basic/call_ref.js"],
    tags = ["basic"],
)

ts_indexer_test(
    name = "dict_fn",
    srcs = ["basic/dict_fn.js"],
    tags = ["basic"],
)

test_suite(
    name = "ts_tests",
    tags = ["ts"],
)

ts_indexer_test(
    name = "function_def_d",
    srcs = [
        "ts/function_def.d.ts",
        "ts/function_def.js",
    ],
    tags = ["ts"],
)

test_suite(
    name = "commonjs_tests",
    tags = ["commonjs"],
)

ts_indexer_test(
    name = "require_object",
    srcs = [
        "commonjs/export_object.js",
        "commonjs/require_object.js",
    ],
    needs_node = True,
    tags = ["commonjs"],
)

ts_indexer_test(
    name = "require_npm",
    srcs = [
        "commonjs/node_modules/mod_from_npm/mod_from_npm_main.js",
        "commonjs/node_modules/mod_from_npm/package.json",
        "commonjs/require_npm.js",
    ],
    needs_node = True,
    tags = ["commonjs"],
)

test_suite(
    name = "repo_tests",
    tags = ["repo"],
)

ts_indexer_test(
    name = "repo_index",
    srcs = [
        "repo/index.js",
    ],
    needs_node = True,
    tags = ["repo"],
    deps = [":repo_files"],
)
