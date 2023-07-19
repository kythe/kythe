load("@aspect_bazel_lib//lib:write_source_files.bzl", "write_source_file", "write_source_files")
load("@aspect_bazel_lib//lib:copy_to_directory.bzl", "copy_to_directory")
load("@io_bazel_rules_go//proto:def.bzl", _go_proto_library = "go_proto_library")
load("@rules_rust//proto:proto.bzl", _rust_proto_library = "rust_proto_library")
load("@rules_rust//rust:rust_common.bzl", "CrateInfo")

KYTHE_IMPORT_BASE = "kythe.io/kythe/proto"

def go_proto_library(proto = None, deps = [], importpath = None, visibility = None, suggested_update_target = "//kythe/proto:update"):
    """Helper for go_proto_library for kythe project.

    A shorthand for a go_proto_library with its import path set to the
    expected default for the Kythe project, e.g.,

      go_kythe_proto(
         proto = ":some_proto",
         deps = ["x", "y", "z"],
      )

    is equivalent in meaning to

      go_proto_library(
         name = "some_go_proto"
         proto = ":some_proto"
         importpath = "kythe.io/kythe/proto/some_proto"
         deps = ["x", "y", "z"],
      )

    Args:
      proto: the proto lib to build a _go_proto lib for
      deps: the deps for the proto lib
    """
    base = proto.rsplit(":", 2)[-1]
    filename = "_".join(base.split("_")[:-1]) + ".pb.go"
    if base.endswith("_proto"):
        name = base[:-len("proto")] + "go_proto"
    else:
        name = base + "_go_proto"

    if not importpath:
        importpath = KYTHE_IMPORT_BASE + "/" + name
    _go_proto_library(
        name = name,
        deps = deps,
        importpath = importpath,
        proto = proto,
        visibility = visibility,
    )
    native.filegroup(
        name = name + "_src",
        output_group = "go_generated_srcs",
        srcs = [name],
    )
    write_source_file(
        name = name + "_sync",
        in_file = name + "_src",
        out_file = name + "/" + filename,
        suggested_update_target = suggested_update_target,
    )

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

def update_generated_protos(name):
    write_source_files(
        name = name,
        additional_update_targets = [
            key
            for key, value in native.existing_rules().items()
            if value["kind"] in ("write_source_file", "write_source_files")
        ],
    )
