load("@io_bazel_rules_go//proto:def.bzl", "go_proto_library")

package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # Apache 2.0

exports_files(["LICENSE"])

go_proto_library(
    name = "records_metadata_go_proto",
    importpath = "kythe.io/third_party/riegeli/records_metadata_go_proto",
    proto = "@com_google_riegeli//riegeli/records:records_metadata_proto",
)
