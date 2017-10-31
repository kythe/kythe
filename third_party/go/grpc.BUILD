package(default_visibility = ["@//visibility:public"])

load("@//third_party:go/build.bzl", "external_go_package")

licenses(["notice"])

exports_files(["LICENSE"])

external_go_package(
    name = "status",
    base_pkg = "google.golang.org/grpc",
    deps = [
        ":codes",
        "@go_genproto//:googleapis/rpc/status",
        "@go_protobuf//:proto",
        "@go_protobuf//:ptypes",
    ],
)

external_go_package(
    name = "codes",
    base_pkg = "google.golang.org/grpc",
)
