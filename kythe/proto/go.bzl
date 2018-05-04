"""This module provides rules for building protos for golang."""

load("@io_bazel_rules_go//proto:def.bzl", "go_proto_library")

KYTHE_IMPORT_BASE = "kythe.io/kythe/proto"

def go_kythe_proto(proto=None, deps=[]):
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
  if base.endswith("_proto"):
    name = base[:-len("proto")] + "go_proto"
  else:
    name = base + "_go_proto"

  go_proto_library(
      name = name,
      deps = deps,
      importpath = KYTHE_IMPORT_BASE + "/" + name,
      proto = proto,
  )
