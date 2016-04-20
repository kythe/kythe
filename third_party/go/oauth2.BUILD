package(default_visibility = ["@//visibility:public"])

load("@//third_party:go/build.bzl", "external_go_package")

licenses(["notice"])

exports_files(["LICENSE"])

external_go_package(
    name = "google",
    base_pkg = "golang.org/x/oauth2",
    exclude_srcs = [
        "appengine_hook.go",
        "appenginevm_hook.go",
    ],
    deps = [
        "@go_gcloud//:compute/metadata",
        "@go_x_net//:context",
        ":internal",
        ":jws",
        ":jwt",
        ":oauth2",
    ],
)

external_go_package(
    name = "jws",
    base_pkg = "golang.org/x/oauth2",
)

external_go_package(
    base_pkg = "golang.org/x/oauth2",
    exclude_srcs = ["client_appengine.go"],
    deps = [
        "@go_x_net//:context",
        ":internal",
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
        "@go_x_net//:context",
        ":internal",
        ":jws",
        ":oauth2",
    ],
)
