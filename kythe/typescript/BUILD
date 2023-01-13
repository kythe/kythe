load("@npm//:defs.bzl", "npm_link_all_packages")
load("@aspect_rules_ts//ts:defs.bzl", "ts_config", "ts_project")
load("@aspect_rules_jasmine//jasmine:defs.bzl", "jasmine_test")
load("@aspect_rules_js//js:defs.bzl", "js_test")

package(default_visibility = ["//kythe:default_visibility"])

ts_project(
    name = "kythe",
    srcs = ["kythe.ts"],
    declaration = True,
    source_map = True,
)

ts_project(
    name = "indexer",
    srcs = [
        "indexer.ts",
        "utf8.ts",
    ],
    declaration = True,
    source_map = True,
    deps = [
        ":kythe",
        "//:node_modules/@types/node",
        "//:node_modules/typescript",
    ],
)

ts_project(
    name = "test_ts",
    srcs = ["test.ts"],
    declaration = True,
    source_map = True,
    deps = [
        ":indexer",
        ":kythe",
        "//:node_modules/@types/node",
        "//:node_modules/source-map-support",
        "//:node_modules/typescript",
    ],
)

ts_project(
    name = "utf8_test_ts",
    testonly = True,
    srcs = ["utf8_test.ts"],
    declaration = True,
    source_map = True,
    deps = [
        ":indexer",
        "//:node_modules/@types/jasmine",
        "//:node_modules/typescript",
    ],
)

DISABLED_TESTS = []

js_test(
    name = "js_indexer_test",
    data = [
        ":test_ts",
    ] + glob(
        ["testdata/**/*"],
        exclude = DISABLED_TESTS,
    ) + [
        "//kythe/go/platform/tools/entrystream",
        "//kythe/cxx/verifier",
    ],
    entry_point = ":test.js",
)

jasmine_test(
    name = "utf8_test",
    args = ["utf8_test.js"],
    chdir = package_name(),
    data = [
        "utf8_test.js",
        ":utf8_test_ts",
    ],
)
