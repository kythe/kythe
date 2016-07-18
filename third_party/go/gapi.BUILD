package(default_visibility = ["@//visibility:public"])

load("@//third_party:go/build.bzl", "external_go_package")

licenses(["notice"])

exports_files(["LICENSE"])

external_go_package(
    name = "googleapi",
    base_pkg = "google.golang.org/api",
    deps = [
        ":googleapi/internal/uritemplates",
        "@go_x_net//:context",
        "@go_x_net//:context/ctxhttp",
    ],
)

external_go_package(
    name = "googleapi/internal/uritemplates",
    base_pkg = "google.golang.org/api",
)

external_go_package(
    name = "transport",
    base_pkg = "google.golang.org/api",
    exclude_srcs = ["dial_appengine.go"],
    deps = [
        ":internal",
        ":option",
        "@go_grpc//:credentials",
        "@go_grpc//:credentials/oauth",
        "@go_grpc//:grpc",
        "@go_x_net//:context",
        "@go_x_oauth2//:google",
        "@go_x_oauth2//:oauth2",
    ],
)

external_go_package(
    name = "option",
    base_pkg = "google.golang.org/api",
    deps = [
        ":internal",
        "@go_grpc//:grpc",
        "@go_x_oauth2//:oauth2",
    ],
)

external_go_package(
    name = "internal",
    base_pkg = "google.golang.org/api",
    deps = [
        "@go_grpc//:grpc",
        "@go_x_oauth2//:oauth2",
    ],
)

external_go_package(
    name = "storage/v1",
    base_pkg = "google.golang.org/api",
    deps = [
        ":gensupport",
        ":googleapi",
        "@go_x_net//:context",
        "@go_x_net//:context/ctxhttp",
    ],
)

external_go_package(
    name = "gensupport",
    base_pkg = "google.golang.org/api",
    deps = [
        ":googleapi",
        "@go_x_net//:context",
        "@go_x_net//:context/ctxhttp",
    ],
)
