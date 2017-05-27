package(default_visibility = ["@//visibility:public"])

load("@io_kythe//third_party:go/build.bzl", "external_go_package")

licenses(["notice"])

exports_files(["LICENSE"])

external_go_package(
    name = "compute/metadata",
    base_pkg = "cloud.google.com/go",
    deps = [
        ":internal",
        "@go_x_net//:context",
        "@go_x_net//:context/ctxhttp",
    ],
)

external_go_package(
    base_pkg = "cloud.google.com/go",
    deps = [
        ":internal",
        "@go_gapi//:option",
        "@go_grpc//:grpc",
        "@go_x_net//:context",
        "@go_x_oauth2//:oauth2",
    ],
)

external_go_package(
    name = "storage",
    base_pkg = "cloud.google.com/go",
    deps = [
        ":go",
        ":internal",
        "@go_gapi//:googleapi",
        "@go_gapi//:iterator",
        "@go_gapi//:option",
        "@go_gapi//:storage/v1",
        "@go_gapi//:transport",
        "@go_x_net//:context",
    ],
)

external_go_package(
    name = "internal",
    base_pkg = "cloud.google.com/go",
    deps = ["@go_x_net//:context"],
)
