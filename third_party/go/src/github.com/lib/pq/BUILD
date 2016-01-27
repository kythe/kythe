package(default_visibility = ["//visibility:public"])

load("/third_party/go/build", "gopath_package")

licenses(["notice"])

gopath_package(
    exclude_srcs = ["*_windows.go"],
    deps = ["//third_party/go/src/github.com/lib/pq/oid"],
)
