package(default_visibility = ["//visibility:public"])

load("/tools/build_rules/go", "go_binary")
load("build", "package")

# TODO(schroederc): split libraries into separate BUILD files once
#     https://github.com/google/bazel/issues/118 is fixed
licenses(["notice"])

filegroup(
    name = "licenses",
    srcs = glob(
        [
            "**/*LICENSE*",
            "**/COPYING*",
            "**/license*",
            "**/copying*",
        ],
    ),
)

package(
    name = "go-diff",
    package = "github.com/sergi/go-diff/diffmatchpatch",
)

package(
    name = "levigo",
    cc_deps = ["//third_party/leveldb"],
    go_build = 1,
    package = "github.com/jmhodges/levigo",
)

package(
    name = "go-sqlite3",
    go_build = 1,
    package = "github.com/mattn/go-sqlite3",
)

package(
    name = "uuid",
    package = "code.google.com/p/go-uuid/uuid",
)

go_binary(
    name = "protoc-gen-go",
    srcs = [
        "src/github.com/golang/protobuf/protoc-gen-go/link_grpc.go",
        "src/github.com/golang/protobuf/protoc-gen-go/main.go",
    ],
    deps = [
        ":protobuf",
        ":protoc-gen-go_generator",
        ":protoc-gen-go_internal_grpc",
    ],
)

package(
    name = "protoc-gen-go_generator",
    package = "github.com/golang/protobuf/protoc-gen-go/generator",
    deps = [
        ":protobuf",
        ":protoc-gen-go_descriptor",
        ":protoc-gen-go_plugin",
    ],
)

package(
    name = "protoc-gen-go_plugin",
    package = "github.com/golang/protobuf/protoc-gen-go/plugin",
    deps = [
        ":protobuf",
        ":protoc-gen-go_descriptor",
    ],
)

package(
    name = "protoc-gen-go_descriptor",
    package = "github.com/golang/protobuf/protoc-gen-go/descriptor",
    deps = [":protobuf"],
)

package(
    name = "protoc-gen-go_internal_grpc",
    package = "github.com/golang/protobuf/protoc-gen-go/internal/grpc",
    deps = [
        ":protoc-gen-go_descriptor",
        ":protoc-gen-go_generator",
    ],
)

package(
    name = "protobuf",
    exclude_srcs = ["*_unsafe.go"],
    package = "github.com/golang/protobuf/proto",
)

package(
    name = "encoding_htmlindex",
    exclude_srcs = ["gen.go"],
    package = "golang.org/x/text/encoding/htmlindex",
    deps = [
        ":encoding",
        ":encoding_charmap",
        ":encoding_internal",
        ":encoding_internal_identifier",
        ":encoding_japanese",
        ":encoding_korean",
        ":encoding_simplifiedchinese",
        ":encoding_traditionalchinese",
        ":encoding_unicode",
        ":text_language",
    ],
)

package(
    name = "text_language",
    exclude_srcs = [
        "gen_*.go",
        "go1_1.go",
        "maketables.go",
    ],
    package = "golang.org/x/text/language",
    deps = [":text_internal_tag"],
)

package(
    name = "text_internal_tag",
    package = "golang.org/x/text/internal/tag",
)

package(
    name = "encoding",
    package = "golang.org/x/text/encoding",
    deps = [":transform"],
)

package(
    name = "encoding_charmap",
    exclude_srcs = ["maketables.go"],
    package = "golang.org/x/text/encoding/charmap",
    deps = [
        ":encoding",
        ":encoding_internal",
        ":encoding_internal_identifier",
        ":transform",
    ],
)

package(
    name = "transform",
    package = "golang.org/x/text/transform",
)

package(
    name = "encoding_unicode",
    package = "golang.org/x/text/encoding/unicode",
    deps = [
        ":encoding",
        ":encoding_internal_identifier",
        ":transform",
    ],
)

package(
    name = "encoding_japanese",
    exclude_srcs = ["maketables.go"],
    package = "golang.org/x/text/encoding/japanese",
    deps = [
        ":encoding",
        ":encoding_internal",
        ":encoding_internal_identifier",
        ":transform",
    ],
)

package(
    name = "encoding_korean",
    exclude_srcs = ["maketables.go"],
    package = "golang.org/x/text/encoding/korean",
    deps = [
        ":encoding",
        ":encoding_internal",
        ":encoding_internal_identifier",
        ":transform",
    ],
)

package(
    name = "encoding_simplifiedchinese",
    exclude_srcs = ["maketables.go"],
    package = "golang.org/x/text/encoding/simplifiedchinese",
    deps = [
        ":encoding",
        ":encoding_internal",
        ":encoding_internal_identifier",
        ":transform",
    ],
)

package(
    name = "encoding_traditionalchinese",
    exclude_srcs = ["maketables.go"],
    package = "golang.org/x/text/encoding/traditionalchinese",
    deps = [
        ":encoding",
        ":encoding_internal",
        ":encoding_internal_identifier",
        ":transform",
    ],
)

package(
    name = "encoding_internal_identifier",
    exclude_srcs = ["gen.go"],
    package = "golang.org/x/text/encoding/internal/identifier",
)

package(
    name = "encoding_internal",
    package = "golang.org/x/text/encoding/internal",
    deps = [
        ":encoding",
        ":encoding_internal_identifier",
        ":transform",
    ],
)

package(
    name = "context",
    package = "golang.org/x/net/context",
)

package(
    name = "net_internal_timeseries",
    package = "golang.org/x/net/internal/timeseries",
)

package(
    name = "trace",
    package = "golang.org/x/net/trace",
    deps = [
        ":context",
        ":net_internal_timeseries",
    ],
)

package(
    name = "html",
    package = "golang.org/x/net/html",
    deps = [":html_atom"],
)

package(
    name = "html_atom",
    exclude_srcs = ["gen.go"],
    package = "golang.org/x/net/html/atom",
)

package(
    name = "cloud",
    package = "google.golang.org/cloud",
    deps = [
        ":cloud_internal",
        ":context",
    ],
)

package(
    name = "cloud_compute_metadata",
    package = "google.golang.org/cloud/compute/metadata",
    deps = [":cloud_internal"],
)

package(
    name = "cloud_storage",
    package = "google.golang.org/cloud/storage",
    deps = [
        ":api_googleapi",
        ":api_storage_v1",
        ":cloud_internal",
        ":context",
    ],
)

package(
    name = "cloud_internal",
    package = "google.golang.org/cloud/internal",
    deps = [":context"],
)

package(
    name = "api_storage_v1",
    package = "google.golang.org/api/storage/v1",
    deps = [
        ":api_googleapi",
        ":context",
    ],
)

package(
    name = "api_googleapi",
    package = "google.golang.org/api/googleapi",
    deps = [
        ":api_googleapi_internal_uritemplates",
        ":context",
    ],
)

package(
    name = "api_googleapi_internal_uritemplates",
    package = "google.golang.org/api/googleapi/internal/uritemplates",
)

package(
    name = "grpc",
    package = "google.golang.org/grpc",
    deps = [
        ":context",
        ":grpc_codes",
        ":grpc_credentials",
        ":grpc_grpclog",
        ":grpc_metadata",
        ":grpc_transport",
        ":protobuf",
        ":trace",
    ],
)

package(
    name = "grpc_codes",
    package = "google.golang.org/grpc/codes",
)

package(
    name = "grpc_metadata",
    package = "google.golang.org/grpc/metadata",
    deps = [":context"],
)

package(
    name = "grpc_transport",
    package = "google.golang.org/grpc/transport",
    deps = [
        ":context",
        ":grpc_codes",
        ":grpc_credentials",
        ":grpc_grpclog",
        ":grpc_metadata",
        ":http2",
        ":http2_hpack",
    ],
)

package(
    name = "grpc_grpclog",
    package = "google.golang.org/grpc/grpclog",
)

package(
    name = "http2",
    package = "github.com/bradfitz/http2",
    deps = [":http2_hpack"],
)

package(
    name = "http2_hpack",
    package = "github.com/bradfitz/http2/hpack",
)

package(
    name = "grpc_credentials",
    package = "google.golang.org/grpc/credentials",
    deps = [
        ":context",
        ":oauth2",
        ":oauth2_google",
        ":oauth2_jwt",
    ],
)

package(
    name = "oauth2",
    exclude_srcs = ["client_appengine.go"],
    package = "golang.org/x/oauth2",
    deps = [
        ":context",
        ":oauth2_internal",
    ],
)

package(
    name = "oauth2_google",
    exclude_srcs = ["appengine_hook.go"],
    package = "golang.org/x/oauth2/google",
    deps = [
        ":cloud_compute_metadata",
        ":context",
        ":oauth2",
        ":oauth2_internal",
        ":oauth2_jwt",
    ],
)

package(
    name = "oauth2_jwt",
    package = "golang.org/x/oauth2/jwt",
    deps = [
        ":context",
        ":oauth2",
        ":oauth2_internal",
        ":oauth2_jws",
    ],
)

package(
    name = "oauth2_jws",
    package = "golang.org/x/oauth2/jws",
)

package(
    name = "oauth2_internal",
    package = "golang.org/x/oauth2/internal",
    deps = [":context"],
)

package(
    name = "benchmark_parse",
    package = "golang.org/x/tools/benchmark/parse",
)
