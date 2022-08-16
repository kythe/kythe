"""This module provides rules for building protos for golang."""

load("@io_bazel_rules_go//go:def.bzl", _GoSource = "GoSource")
load("@io_bazel_rules_go//proto:def.bzl", "go_proto_library")
load("//tools:build_rules/testing.bzl", "file_diff_test")

KYTHE_IMPORT_BASE = "kythe.io/kythe/proto"

def _go_proto_src_impl(ctx):
    """Copy the generated source of a go_proto_library."""
    lib = ctx.attr.library
    for src in lib.actions[0].outputs.to_list():
        ctx.actions.symlink(
            output = ctx.outputs.generated,
            target_file = src,
        )
        break

_go_proto_src = rule(
    _go_proto_src_impl,
    attrs = {
        "library": attr.label(
            providers = [_GoSource],
            allow_single_file = True,
            mandatory = True,
        ),
    },
    outputs = {
        "generated": "%{name}.cmp",
    },
)

def go_kythe_proto(proto = None, deps = [], importpath = None, visibility = None):
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
    go_proto_library(
        name = name,
        deps = deps,
        importpath = importpath,
        proto = proto,
        visibility = visibility,
    )

    # Copy the generated source from the proto library so we can compare it to
    # what we checked in. Nothing useful is ever easy in Starlark.
    _go_proto_src(
        name = name + "_src",
        library = ":" + name,
    )

    # Fail if the generated source differs from the checked-in version.  Prompt
    # the user to re-generate if necessary. This assumes the checked-in source
    # for kythe/proto:foo_go_proto is in kythe/proto/foo_go_proto/foo.pb.go.
    file_diff_test(
        name = name + "_sync_test",
        file1 = ":%s_src" % name,
        file2 = ":%s/%s" % (name, filename),
        message = ("The checked in proto for '%s' is out of sync;" +
                   " please run kythe/proto/generate_protobufs.py") % name,
    )
