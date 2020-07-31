package(default_visibility = ["//visibility:public"])

load("@io_bazel_rules_rust//rust:rust.bzl", "rust_binary", "rust_clippy", "rust_library", "rust_test")
load("@io_bazel_rules_rust//proto:toolchain.bzl", "PROTO_COMPILE_DEPS")

rust_library(
    name = "fuchsia_extractor_lib",
    srcs = glob(
        include = ["src/**/*.rs"],
    ),
    deps = [
      "//kythe/cxx/common:kzip_writer_c_api",
      "//kythe/proto:analysis_rust_proto",
      "//kythe/rust/cargo:anyhow",
      "//kythe/rust/cargo:protobuf",
      "@com_google_absl//absl/status",
    ],
    edition = "2018",
)

rust_test(
    name = "fuchsia_extractor_lib_test",
    crate = ":fuchsia_extractor_lib",
    deps = [
      "//kythe/rust/cargo:tempdir",
    ],
    edition = "2018",
    # See https://github.com/bazelbuild/rules_rust/issues/118.  Without this,
    # rust binaries require their deps to be compiled with -fPIC, which is not
    # always the case. fastbuild and dbg use -fPIC, but opt does not; and rust
    # build rules are not yet sophisticated enough to know the difference.
    rustc_flags = [
        "-Crelocation-model=static",
    ],
)

rust_clippy(
    name = "clippy",
    deps = [
        ":fuchsia_extractor_lib",
    ],
)

