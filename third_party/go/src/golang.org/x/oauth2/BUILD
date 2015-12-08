package(default_visibility = ["//visibility:public"])

load("/third_party/go/build", "gopath_package")

licenses(["notice"])

exports_files(["LICENSE"])

gopath_package(
    exclude_srcs = ["client_appengine.go"],
    deps = [
        "//third_party/go/src/golang.org/x/net/context",
        "//third_party/go/src/golang.org/x/oauth2/internal",
    ],
)
