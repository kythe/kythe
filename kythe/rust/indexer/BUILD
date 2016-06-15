package(default_visibility = ["//kythe:default_visibility"])

load("//tools/build_rules/rust:cargo.bzl", "cargo_lib", "cargo_bin", "cargo_test")

cargo_lib(
    name = "libindexer",
    loc = "kythe/rust/indexer/",
)

cargo_bin(
    name = "indexer",
    loc = "kythe/rust/indexer/",
)

cargo_test(
    name = "test_it_works",
    size = "small",
    test_name = "tests::it_works",
    loc = "kythe/rust/indexer/",
)
