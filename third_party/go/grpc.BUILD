package(default_visibility = ["@//visibility:public"])

load("@//third_party:go/build.bzl", "external_go_package")

licenses(["notice"])

exports_files(["LICENSE"])

external_go_package(
    name = "status",
    base_pkg = "google.golang.org/grpc",
    deps = [
        ":codes",
        "@com_github_golang_protobuf//proto:go_default_library",
        "@com_github_golang_protobuf//ptypes:go_default_library",
        "@go_genproto//:googleapis/rpc/status",
    ],
)

external_go_package(
    name = "codes",
    base_pkg = "google.golang.org/grpc",
)
