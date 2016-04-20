package(default_visibility = ["@//visibility:public"])

load("@//third_party:go/build.bzl", "external_go_package")

licenses(["notice"])

exports_files(["LICENSE"])

external_go_package(
    name = "language",
    base_pkg = "golang.org/x/text",
    exclude_srcs = [
        "gen_*.go",
        "go1_1.go",
        "maketables.go",
    ],
    deps = [":internal/tag"],
)

external_go_package(
    name = "transform",
    base_pkg = "golang.org/x/text",
)

external_go_package(
    name = "runes",
    base_pkg = "golang.org/x/text",
    deps = [":transform"],
)

external_go_package(
    name = "encoding/traditionalchinese",
    base_pkg = "golang.org/x/text",
    exclude_srcs = ["maketables.go"],
    deps = [
        ":encoding",
        ":encoding/internal",
        ":encoding/internal/identifier",
        ":transform",
    ],
)

external_go_package(
    name = "encoding/unicode",
    base_pkg = "golang.org/x/text",
    deps = [
        ":encoding",
        ":encoding/internal",
        ":encoding/internal/identifier",
        ":internal/utf8internal",
        ":runes",
        ":transform",
    ],
)

external_go_package(
    name = "encoding/htmlindex",
    base_pkg = "golang.org/x/text",
    exclude_srcs = ["gen.go"],
    deps = [
        ":encoding",
        ":encoding/charmap",
        ":encoding/internal",
        ":encoding/internal/identifier",
        ":encoding/japanese",
        ":encoding/korean",
        ":encoding/simplifiedchinese",
        ":encoding/traditionalchinese",
        ":encoding/unicode",
        ":language",
    ],
)

external_go_package(
    name = "encoding/charmap",
    base_pkg = "golang.org/x/text",
    exclude_srcs = ["maketables.go"],
    deps = [
        ":encoding",
        ":encoding/internal",
        ":encoding/internal/identifier",
        ":transform",
    ],
)

external_go_package(
    name = "encoding",
    base_pkg = "golang.org/x/text",
    deps = [
        ":encoding/internal/identifier",
        ":transform",
    ],
)

external_go_package(
    name = "encoding/simplifiedchinese",
    base_pkg = "golang.org/x/text",
    exclude_srcs = ["maketables.go"],
    deps = [
        ":encoding",
        ":encoding/internal",
        ":encoding/internal/identifier",
        ":transform",
    ],
)

external_go_package(
    name = "encoding/korean",
    base_pkg = "golang.org/x/text",
    exclude_srcs = ["maketables.go"],
    deps = [
        ":encoding",
        ":encoding/internal",
        ":encoding/internal/identifier",
        ":transform",
    ],
)

external_go_package(
    name = "encoding/japanese",
    base_pkg = "golang.org/x/text",
    exclude_srcs = ["maketables.go"],
    deps = [
        ":encoding",
        ":encoding/internal",
        ":encoding/internal/identifier",
        ":transform",
    ],
)

external_go_package(
    name = "encoding/internal",
    base_pkg = "golang.org/x/text",
    deps = [
        ":encoding",
        ":encoding/internal/identifier",
        ":transform",
    ],
)

external_go_package(
    name = "encoding/internal/identifier",
    base_pkg = "golang.org/x/text",
    exclude_srcs = ["gen.go"],
)

external_go_package(
    name = "internal/utf8internal",
    base_pkg = "golang.org/x/text",
)

external_go_package(
    name = "internal/tag",
    base_pkg = "golang.org/x/text",
)
