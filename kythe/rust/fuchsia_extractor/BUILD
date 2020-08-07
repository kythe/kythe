package(default_visibility = ["//visibility:public"])

load("@io_bazel_rules_rust//rust:rust.bzl", "rust_binary", "rust_clippy", "rust_library", "rust_test")
load("@io_bazel_rules_rust//proto:toolchain.bzl", "PROTO_COMPILE_DEPS")

rust_library(
    name = "fuchsia_extractor_lib",
    srcs = glob(
        include = ["src/**/*.rs"],
        exclude = ["src/bin/**/*.rs"],
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
    #
    # Using static relocation model will sometimes cause the binary to crash in
    # opt and fastbuild mode, so we use this flag only in opt.
    rustc_flags = select({
        ":if_opt": [ "-Crelocation-model=static" ],
        "//conditions:default": [],
    })
)

rust_binary(
    edition = "2018",
    name = "fuchsia_extractor",
    srcs = glob(["src/bin/**/*.rs"]),
    deps = [
        ":fuchsia_extractor_lib",
        "//kythe/proto:analysis_rust_proto",
        "//kythe/rust/cargo:anyhow",
        "//kythe/rust/cargo:clap",
        "//kythe/rust/cargo:lazy_static",
        "//kythe/rust/cargo:protobuf",
        "//kythe/rust/cargo:regex",
        "//kythe/rust/cargo:rls_data",
        "//kythe/rust/cargo:rust_crypto",
        "//kythe/rust/cargo:serde_json",
    ],
    rustc_flags = select({
        ":if_opt": [ "-Crelocation-model=static" ],
        "//conditions:default": [],
    })
)

rust_test(
    name = "fuchsia_extractor_test",
    crate = ":fuchsia_extractor",
    deps = [
      "//kythe/rust/cargo:zip",
    ],
    edition = "2018",
    # See https://github.com/bazelbuild/rules_rust/issues/118.  Without this,
    # rust binaries require their deps to be compiled with -fPIC, which is not
    # always the case. fastbuild and dbg use -fPIC, but opt does not; and rust
    # build rules are not yet sophisticated enough to know the difference.
    #
    # Using static relocation model will sometimes cause the binary to crash in
    # opt and fastbuild mode, so we use this flag only in opt.
    rustc_flags = select({
        ":if_opt": [ "-Crelocation-model=static" ],
        "//conditions:default": [],
    }),
    data = ["//kythe/rust/fuchsia_extractor/testdata"],
)

rust_clippy(
    name = "clippy",
    deps = [
        ":fuchsia_extractor_lib",
    ],
)

config_setting(
    name = "if_opt",
    values = {
        "compilation_mode": "opt",
    },
)

