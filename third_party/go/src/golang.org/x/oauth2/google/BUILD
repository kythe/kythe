package(default_visibility = ["//visibility:public"])

load("//third_party/go:build.bzl", "gopath_package")

licenses(["notice"])

gopath_package(
    exclude_srcs = [
        "appengine_hook.go",
        "appenginevm_hook.go",
    ],
    deps = [
        "//third_party/go/src/golang.org/x/net/context",
        "//third_party/go/src/golang.org/x/oauth2",
        "//third_party/go/src/golang.org/x/oauth2/internal",
        "//third_party/go/src/golang.org/x/oauth2/jws",
        "//third_party/go/src/golang.org/x/oauth2/jwt",
        "//third_party/go/src/google.golang.org/cloud/compute/metadata",
    ],
)
