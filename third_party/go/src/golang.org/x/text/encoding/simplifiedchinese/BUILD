package(default_visibility = ["//visibility:public"])

load("//third_party/go:build.bzl", "gopath_package")

licenses(["notice"])

gopath_package(
    exclude_srcs = ["maketables.go"],
    deps = [
        "//third_party/go/src/golang.org/x/text/encoding",
        "//third_party/go/src/golang.org/x/text/encoding/internal",
        "//third_party/go/src/golang.org/x/text/encoding/internal/identifier",
        "//third_party/go/src/golang.org/x/text/transform",
    ],
)
