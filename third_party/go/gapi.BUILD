package(default_visibility = ["@//visibility:public"])

load("@//third_party:go/build.bzl", "external_go_package")

licenses(["notice"])

exports_files(["LICENSE"])

external_go_package(
    name = "googleapi",
    base_pkg = "google.golang.org/api",
    deps = [
        "@go_x_net//:context",
        "@go_x_net//:context/ctxhttp",
        ":googleapi/internal/uritemplates",
    ],
)

external_go_package(
    name = "googleapi/internal/uritemplates",
    base_pkg = "google.golang.org/api",
)

external_go_package(
    name = "storage/v1",
    base_pkg = "google.golang.org/api",
    deps = [
        "@go_x_net//:context",
        "@go_x_net//:context/ctxhttp",
        ":gensupport",
        ":googleapi",
    ],
)

external_go_package(
    name = "gensupport",
    base_pkg = "google.golang.org/api",
    deps = [
        "@go_x_net//:context",
        "@go_x_net//:context/ctxhttp",
        ":googleapi",
    ],
)
