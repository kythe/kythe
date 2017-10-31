package(default_visibility = ["@//visibility:public"])

load("@//third_party:go/build.bzl", "external_go_package")

licenses(["notice"])

exports_files(["LICENSE"])

external_go_package(
    name = "googleapis/rpc/status",
    base_pkg = "google.golang.org/genproto",
    deps = [
        "@go_protobuf//:proto",
        "@go_protobuf//:ptypes/any",
    ],
)
