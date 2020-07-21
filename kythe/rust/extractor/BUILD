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
    cmd = "echo \"Log available at $@\" && LD_LIBRARY_PATH=external/rust_linux_x86_64/lib TEST_FILE=\"$(SRCS)\" ./$(location :extractor_test_binary) > $@ && cat $@",
    srcs = [
        "testfiles/main.rs",
    ],
    tools = [":extractor_test_binary"],
    outs = ["output.txt"],
    testonly = True,
    local = True,
)
