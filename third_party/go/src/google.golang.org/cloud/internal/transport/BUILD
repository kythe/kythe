package(default_visibility = ["//visibility:public"])

load("/third_party/go/build", "gopath_package")

licenses(["notice"])

gopath_package(
    exclude_srcs = ["cancelreq_legacy.go"],
    deps = [
        "//third_party/go/src/github.com/golang/protobuf/proto",
        "//third_party/go/src/golang.org/x/net/context",
        "//third_party/go/src/golang.org/x/oauth2",
        "//third_party/go/src/golang.org/x/oauth2/google",
        "//third_party/go/src/google.golang.org/cloud",
        "//third_party/go/src/google.golang.org/cloud/internal/opts",
        "//third_party/go/src/google.golang.org/grpc",
        "//third_party/go/src/google.golang.org/grpc/credentials",
        "//third_party/go/src/google.golang.org/grpc/credentials/oauth",
    ],
)
