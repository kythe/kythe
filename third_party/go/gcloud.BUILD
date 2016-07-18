package(default_visibility = ["@//visibility:public"])

load("@//third_party:go/build.bzl", "external_go_package")

licenses(["notice"])

exports_files(["LICENSE"])

external_go_package(
    name = "compute/metadata",
    base_pkg = "google.golang.org/cloud",
    deps = [
        ":internal",
        "@go_x_net//:context",
        "@go_x_net//:context/ctxhttp",
    ],
)

external_go_package(
    base_pkg = "google.golang.org/cloud",
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
    base_pkg = "google.golang.org/cloud",
    deps = [
        ":cloud",
        ":internal",
        ":internal/transport",
        "@go_gapi//:googleapi",
        "@go_gapi//:storage/v1",
        "@go_x_net//:context",
    ],
)

external_go_package(
    name = "internal",
    base_pkg = "google.golang.org/cloud",
    deps = ["@go_x_net//:context"],
)

external_go_package(
    name = "internal/transport",
    base_pkg = "google.golang.org/cloud",
    exclude_srcs = ["cancelreq_legacy.go"],
    deps = [
        ":cloud",
        "@go_gapi//:option",
        "@go_gapi//:transport",
        "@go_grpc//:credentials",
        "@go_grpc//:credentials/oauth",
        "@go_grpc//:grpc",
        "@go_protobuf//:proto",
        "@go_x_net//:context",
        "@go_x_oauth2//:google",
        "@go_x_oauth2//:oauth2",
    ],
)
