load("@aspect_bazel_lib//lib:write_source_files.bzl", "write_source_file")
load("@aspect_bazel_lib//lib:copy_to_directory.bzl", "copy_to_directory")
load("@rules_rust//proto/protobuf:defs.bzl", _rust_proto_library = "rust_proto_library")
load("@rules_rust//rust:rust_common.bzl", "CrateInfo")

def _rust_proto_sources_impl(ctx):
    return [DefaultInfo(files = ctx.attr.crate[CrateInfo].srcs)]

_rust_proto_sources = rule(
    implementation = _rust_proto_sources_impl,
    attrs = {
        "crate": attr.label(
            mandatory = True,
            providers = [CrateInfo],
        ),
        "out": attr.string(),
    },
)

def rust_proto_library(name, **kwargs):
    _rust_proto_library(name = name, **kwargs)
    _rust_proto_sources(
        name = name + "_src",
        crate = name,
    )
    copy_to_directory(
        name = name + "_dir",
        srcs = [name + "_src"],
        out = name,
        replace_prefixes = {"*/": ""},
    )
    write_source_file(
        name = name + "_sync",
        in_file = name + "_dir",
        out_file = name,
        # The use of the same name for the source directory
        # and rust_proto_library rule target precludes automated testing.
        diff_test = False,
    )
