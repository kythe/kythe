package(default_visibility = ["@//visibility:public"])

load("@io_kythe//third_party:go/build.bzl", "external_go_package")

licenses(["notice"])

exports_files(["LICENSE"])

external_go_package(
    name = "http2",
    base_pkg = "golang.org/x/net",
    deps = [
        ":http2/hpack",
        ":idna",
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
    deps = [":idna"],
)

external_go_package(
    name = "context",
    base_pkg = "golang.org/x/net",
)

external_go_package(
    name = "context/ctxhttp",
    base_pkg = "golang.org/x/net",
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
)

external_go_package(
    name = "internal/timeseries",
    base_pkg = "golang.org/x/net",
)

external_go_package(
    name = "idna",
    base_pkg = "golang.org/x/net",
)
