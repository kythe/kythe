package(default_visibility = ["@//visibility:public"])

load("@io_kythe//third_party:go/build.bzl", "external_go_package")

licenses(["notice"])

exports_files(["LICENSE"])

external_go_package(
    name = "google",
    base_pkg = "golang.org/x/oauth2",
    deps = [
        ":internal",
        ":jws",
        ":jwt",
        ":oauth2",
        "@go_gcloud//:compute/metadata",
        "@go_x_net//:context",
    ],
)

external_go_package(
    name = "jws",
    base_pkg = "golang.org/x/oauth2",
)

external_go_package(
    base_pkg = "golang.org/x/oauth2",
    deps = [
        ":internal",
        "@go_x_net//:context",
    ],
)

external_go_package(
    name = "internal",
    base_pkg = "golang.org/x/oauth2",
    deps = ["@go_x_net//:context"],
)

external_go_package(
    name = "jwt",
    base_pkg = "golang.org/x/oauth2",
    deps = [
        ":internal",
        ":jws",
        ":oauth2",
        "@go_x_net//:context",
    ],
)
