package(default_visibility = ["@//visibility:public"])

load("@//third_party:go/build.bzl", "external_go_package")

licenses(["notice"])

exports_files(["LICENSE"])

external_go_package(
    name = "http2",
    base_pkg = "golang.org/x/net",
    exclude_srcs = [
        "go15.go",
        "not_go16.go",
        "go17.go",
    ],
    deps = [
        ":http2/hpack",
        ":lex/httplex",
    ],
)

external_go_package(
    name = "http2/hpack",
    base_pkg = "golang.org/x/net",
)

external_go_package(
    name = "lex/httplex",
    base_pkg = "golang.org/x/net",
)

external_go_package(
    name = "context",
    base_pkg = "golang.org/x/net",
    exclude_srcs = [
        "withtimeout_test.go",
        "go17.go",
    ],
)

external_go_package(
    name = "context/ctxhttp",
    base_pkg = "golang.org/x/net",
    exclude_srcs = [
        "cancelreq_go14.go",
        "ctxhttp.go",
    ],
    deps = [":context"],
)

external_go_package(
    name = "html",
    base_pkg = "golang.org/x/net",
    deps = [":html/atom"],
)

external_go_package(
    name = "trace",
    base_pkg = "golang.org/x/net",
    deps = [
        ":context",
        ":internal/timeseries",
    ],
)

external_go_package(
    name = "html/atom",
    base_pkg = "golang.org/x/net",
    exclude_srcs = ["gen.go"],
)

external_go_package(
    name = "internal/timeseries",
    base_pkg = "golang.org/x/net",
)
