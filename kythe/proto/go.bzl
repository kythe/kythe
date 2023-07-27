load("@aspect_bazel_lib//lib:write_source_files.bzl", "write_source_file")
load("@io_bazel_rules_go//proto:def.bzl", _go_proto_library = "go_proto_library")

KYTHE_IMPORT_BASE = "kythe.io/kythe/proto"

def go_proto_library(
        name = None,
        proto = None,
        deps = [],
        importpath = None,
        visibility = None,
        compilers = None,
        suggested_update_target = "//{package}:update"):
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
    if suggested_update_target != None:
        suggested_update_target = suggested_update_target.format(package = native.package_name())

    base = proto.rsplit(":", 2)[-1]
    filename = "_".join(base.split("_")[:-1]) + ".pb.go"
    if name == None:
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
