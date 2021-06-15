load("@rules_rust//rust:rust.bzl", "rust_binary", "rust_clippy", "rust_library", "rust_test")

package(default_visibility = ["//visibility:public"])

rust_library(
    name = "fuchsia_extractor_lib",
    srcs = glob(
        include = ["src/**/*.rs"],
        exclude = ["src/bin/**/*.rs"],
    ),
    edition = "2018",
    deps = [
        "//kythe/cxx/common:kzip_writer_c_api",
        "//kythe/proto:analysis_rust_proto",
        "//kythe/rust/cargo:anyhow",
        "@com_google_absl//absl/status",
        "@rules_rust//proto/raze:protobuf",
    ],
)

rust_test(
    name = "fuchsia_extractor_lib_test",
    crate = ":fuchsia_extractor_lib",
    edition = "2018",
    # See https://github.com/bazelbuild/rules_rust/issues/118.  Without this,
    # rust binaries require their deps to be compiled with -fPIC, which is not
    # always the case. fastbuild and dbg use -fPIC, but opt does not; and rust
    # build rules are not yet sophisticated enough to know the difference.
    #
    # Using static relocation model will sometimes cause the binary to crash in
    # opt and fastbuild mode, so we use this flag only in opt.
    rustc_flags = select({
        ":if_opt": ["-Crelocation-model=static"],
        "//conditions:default": [],
    }),
    deps = [
        "//kythe/rust/cargo:tempdir",
    ],
)

rust_binary(
    name = "fuchsia_extractor",
    srcs = glob(["src/bin/**/*.rs"]),
    edition = "2018",
    rustc_flags = select({
        ":if_opt": ["-Crelocation-model=static"],
        "//conditions:default": [],
    }),
    deps = [
        ":fuchsia_extractor_lib",
        "//kythe/proto:analysis_rust_proto",
        "//kythe/rust/cargo:anyhow",
        "//kythe/rust/cargo:clap",
        "//kythe/rust/cargo:lazy_static",
        "//kythe/rust/cargo:rayon",
        "//kythe/rust/cargo:regex",
        "//kythe/rust/cargo:rls_data",
        "//kythe/rust/cargo:serde_json",
        "//kythe/rust/cargo:tempdir",
        "//kythe/rust/cargo:hex",
        "//kythe/rust/cargo:sha2",
        "@rules_rust//proto/raze:protobuf",
    ],
)

rust_test(
    name = "fuchsia_extractor_test",
    crate = ":fuchsia_extractor",
    data = [
        "//kythe/go/platform/tools/kzip",
        "//kythe/rust/fuchsia_extractor/testdata",
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
        ":if_opt": ["-Crelocation-model=static"],
        "//conditions:default": [],
    }),
    deps = [
        "//kythe/rust/cargo:serial_test",
        "//kythe/rust/cargo:zip",
    ],
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
