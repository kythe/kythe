package(default_visibility = ["//visibility:public"])

load("@io_bazel_rules_rust//rust:rust.bzl", "rust_clippy", "rust_library", "rust_test")

rust_library(
    name = "kythe_rust_extractor",
    srcs = glob(
        include = ["src/**/*.rs"],
        exclude = ["src/bin/**"],
    ),
    edition = "2018",
)

rust_clippy(
    name = "clippy",
    deps = [
        ":kythe_rust_extractor",
    ],
)

rust_test(
    name = "extractor_test_binary",
    srcs = glob(["tests/**/*.rs"]),
    edition = "2018",
    deps = [
        ":kythe_rust_extractor",
        "//kythe/rust/extractor/cargo:tempdir",
    ],
)

genrule(
    name = "extractor_test",
    cmd = "echo \"Log available at $@\" && " +
          select({
              "//:darwin": "LD_LIBRARY_PATH=external/rust_linux_x86_64/lib:external/rust_darwin_x86_64/lib/rustlib/x86_64-apple-darwin/lib SYSROOT=external/rust_darwin_x86_64/lib/rustlib/x86_64-apple-darwin/lib ",
              "//conditions:default": "LD_LIBRARY_PATH=external/rust_linux_x86_64/lib:external/rust_linux_x86_64/lib/rustlib/x86_64-unknown-linux-gnu/lib SYSROOT=external/rust_linux_x86_64/lib/rustlib/x86_64-unknown-linux-gnu/lib ",
          }) +
          "TEST_FILE=\"$(SRCS)\" ./$(location :extractor_test_binary) > $@ && cat $@",
    srcs = [
        "testfiles/main.rs",
    ],
    tools = [":extractor_test_binary"] + select({
        "//:darwin": ["@rust_darwin_x86_64//:rustc_lib", "@rust_darwin_x86_64//:rust_lib-x86_64-apple-darwin"],
        "//conditions:default": ["@rust_linux_x86_64//:rustc_lib", "@rust_linux_x86_64//:rust_lib-x86_64-unknown-linux-gnu"],
    }),
    outs = ["output.txt"],
    testonly = True,
)
