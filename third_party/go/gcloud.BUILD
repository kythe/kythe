package(default_visibility = ["@//visibility:public"])

load("@//third_party:go/build.bzl", "external_go_package")

licenses(["notice"])

exports_files(["LICENSE"])

external_go_package(
    name = "compute/metadata",
    base_pkg = "google.golang.org/cloud",
    deps = [
        "@go_x_net//:context",
        "@go_x_net//:context/ctxhttp",
        ":internal",
    ],
)

external_go_package(
    base_pkg = "google.golang.org/cloud",
    deps = [
        "@//third_party/go/src/google.golang.org/grpc",
        "@go_oauth2//:oauth2",
        "@go_x_net//:context",
        ":internal",
        ":internal/opts",
    ],
)

external_go_package(
    name = "storage",
    base_pkg = "google.golang.org/cloud",
    deps = [
        "@//third_party/go/src/google.golang.org/api/googleapi",
        "@//third_party/go/src/google.golang.org/api/storage/v1",
        "@go_x_net//:context",
        ":cloud",
        ":internal",
        ":internal/transport",
    ],
)

external_go_package(
    name = "internal/opts",
    base_pkg = "google.golang.org/cloud",
    deps = [
        "@//third_party/go/src/google.golang.org/grpc",
        "@go_oauth2//:oauth2",
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
        "@//third_party/go/src/github.com/golang/protobuf/proto",
        "@//third_party/go/src/google.golang.org/grpc",
        "@//third_party/go/src/google.golang.org/grpc/credentials",
        "@//third_party/go/src/google.golang.org/grpc/credentials/oauth",
        "@go_oauth2//:google",
        "@go_oauth2//:oauth2",
        "@go_x_net//:context",
        ":cloud",
        ":internal/opts",
    ],
)
