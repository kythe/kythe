package(default_visibility = ["@//visibility:public"])

load("@io_kythe//third_party:go/build.bzl", "external_go_package")

licenses(["notice"])

exports_files(["LICENSE"])

external_go_package(
    name = "metadata",
    base_pkg = "google.golang.org/grpc",
    deps = ["@go_x_net//:context"],
)

external_go_package(
    name = "grpclog",
    base_pkg = "google.golang.org/grpc",
)

external_go_package(
    base_pkg = "google.golang.org/grpc",
    deps = [
        ":codes",
        ":credentials",
        ":grpclog",
        ":internal",
        ":metadata",
        ":naming",
        ":stats",
        ":tap",
        ":transport",
        "@go_protobuf//:proto",
        "@go_x_net//:context",
        "@go_x_net//:http2",
        "@go_x_net//:trace",
    ],
)

external_go_package(
    name = "codes",
    base_pkg = "google.golang.org/grpc",
)

external_go_package(
    name = "transport",
    base_pkg = "google.golang.org/grpc",
    deps = [
        ":codes",
        ":credentials",
        ":grpclog",
        ":metadata",
        ":peer",
        ":stats",
        ":tap",
        "@go_x_net//:context",
        "@go_x_net//:http2",
        "@go_x_net//:http2/hpack",
        "@go_x_net//:trace",
    ],
)

external_go_package(
    name = "credentials",
    base_pkg = "google.golang.org/grpc",
    deps = [
        "@go_x_net//:context",
        "@go_x_oauth2//:google",
        "@go_x_oauth2//:jwt",
        "@go_x_oauth2//:oauth2",
    ],
)

external_go_package(
    name = "stats",
    base_pkg = "google.golang.org/grpc",
    deps = [
        "@go_x_net//:context",
    ],
)

external_go_package(
    name = "tap",
    base_pkg = "google.golang.org/grpc",
    deps = [
        "@go_x_net//:context",
    ],
)

external_go_package(
    name = "credentials/oauth",
    base_pkg = "google.golang.org/grpc",
    deps = [
        ":credentials",
        "@go_x_net//:context",
        "@go_x_oauth2//:google",
        "@go_x_oauth2//:jwt",
        "@go_x_oauth2//:oauth2",
    ],
)

external_go_package(
    name = "naming",
    base_pkg = "google.golang.org/grpc",
)

external_go_package(
    name = "peer",
    base_pkg = "google.golang.org/grpc",
    deps = [
        ":credentials",
        "@go_x_net//:context",
    ],
)

external_go_package(
    name = "internal",
    base_pkg = "google.golang.org/grpc",
)
