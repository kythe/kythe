load("@rules_rust//rust:defs.bzl", "rust_binary", "rust_clippy", "rust_library", "rust_test")

package(default_visibility = ["//visibility:public"])

rust_library(
    name = "fuchsia_extractor_lib",
    srcs = glob(
        include = ["src/**/*.rs"],
        exclude = ["src/bin/**/*.rs"],
    ),
    edition = "2021",
    deps = [
        "//kythe/cxx/common:kzip_writer_c_api",
        "//kythe/proto:analysis_rust_proto",
        "@com_google_absl//absl/status",
        "@crate_index//:anyhow",
        "@crate_index//:protobuf",
    ],
)

# These broke with the protobuf update, see kythe/kythe#5548
#rust_test(
#    name = "fuchsia_extractor_lib_test",
#    crate = ":fuchsia_extractor_lib",
#    edition = "2021",
#    # See https://github.com/bazelbuild/rules_rust/issues/118.  Without this,
#    # rust binaries require their deps to be compiled with -fPIC, which is not
#    # always the case. fastbuild and dbg use -fPIC, but opt does not; and rust
#    # build rules are not yet sophisticated enough to know the difference.
#    #
#    # Using static relocation model will sometimes cause the binary to crash in
#    # opt and fastbuild mode, so we use this flag only in opt.
#    rustc_flags = select({
#        ":if_opt": ["-Crelocation-model=static"],
#        "//conditions:default": [],
#    }),
#    deps = [
#        "@crate_index//:tempfile",
#    ],
#)

rust_binary(
    name = "fuchsia_extractor",
    srcs = glob(["src/bin/**/*.rs"]),
    edition = "2021",
    rustc_flags = select({
        ":if_opt": ["-Crelocation-model=static"],
        "//conditions:default": [],
    }),
    deps = [
        ":fuchsia_extractor_lib",
        "//kythe/proto:analysis_rust_proto",
        "@crate_index//:anyhow",
        "@crate_index//:clap",
        "@crate_index//:hex",
        "@crate_index//:lazy_static",
        "@crate_index//:protobuf",
        "@crate_index//:rayon",
        "@crate_index//:regex",
        "@crate_index//:rls-data",
        "@crate_index//:serde_json",
        "@crate_index//:sha2",
        "@crate_index//:tempfile",
    ],
)

# These broke with the protobuf update, see kythe/kythe#5548
#rust_test(
#    name = "fuchsia_extractor_test",
#    crate = ":fuchsia_extractor",
#    data = [
#        "//kythe/go/platform/tools/kzip",
#        "//kythe/rust/fuchsia_extractor/testdata",
#    ],
#    edition = "2021",
#    # See https://github.com/bazelbuild/rules_rust/issues/118.  Without this,
#    # rust binaries require their deps to be compiled with -fPIC, which is not
#    # always the case. fastbuild and dbg use -fPIC, but opt does not; and rust
#    # build rules are not yet sophisticated enough to know the difference.
#    #
#    # Using static relocation model will sometimes cause the binary to crash in
#    # opt and fastbuild mode, so we use this flag only in opt.
#    rustc_flags = select({
#        ":if_opt": ["-Crelocation-model=static"],
#        "//conditions:default": [],
#    }),
#    deps = [
#        "@crate_index//:serial_test",
#        "@crate_index//:zip",
#    ],
#)

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
