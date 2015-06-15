package(default_visibility = ["//visibility:public"])

load("/tools/build_rules/go", "go_library", "go_binary")

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

go_library(
    name = "go-diff",
    srcs = [
        "src/github.com/sergi/go-diff/diffmatchpatch/dmp.go",
        "src/github.com/sergi/go-diff/diffmatchpatch/stack.go",
    ],
    package = "github.com/sergi/go-diff/diffmatchpatch",
)

go_library(
    name = "levigo",
    srcs = [
        "src/github.com/jmhodges/levigo/batch.go",
        "src/github.com/jmhodges/levigo/cache.go",
        "src/github.com/jmhodges/levigo/comparator.go",
        "src/github.com/jmhodges/levigo/conv.go",
        "src/github.com/jmhodges/levigo/db.go",
        "src/github.com/jmhodges/levigo/doc.go",
        "src/github.com/jmhodges/levigo/env.go",
        "src/github.com/jmhodges/levigo/filterpolicy.go",
        "src/github.com/jmhodges/levigo/iterator.go",
        "src/github.com/jmhodges/levigo/options.go",
        "src/github.com/jmhodges/levigo/version.go",
    ],
    cc_deps = ["//third_party/leveldb"],
    go_build = 1,
    package = "github.com/jmhodges/levigo",
)

go_library(
    name = "go-sqlite3",
    srcs = [
        "src/github.com/mattn/go-sqlite3/backup.go",
        "src/github.com/mattn/go-sqlite3/doc.go",
        "src/github.com/mattn/go-sqlite3/error.go",
        "src/github.com/mattn/go-sqlite3/sqlite3.go",
        "src/github.com/mattn/go-sqlite3/sqlite3_other.go",
        "src/github.com/mattn/go-sqlite3/sqlite3_windows.go",
    ],
    go_build = 1,
    package = "github.com/mattn/go-sqlite3",
)

go_library(
    name = "uuid",
    srcs = [
        "src/code.google.com/p/go-uuid/uuid/dce.go",
        "src/code.google.com/p/go-uuid/uuid/doc.go",
        "src/code.google.com/p/go-uuid/uuid/hash.go",
        "src/code.google.com/p/go-uuid/uuid/json.go",
        "src/code.google.com/p/go-uuid/uuid/node.go",
        "src/code.google.com/p/go-uuid/uuid/time.go",
        "src/code.google.com/p/go-uuid/uuid/util.go",
        "src/code.google.com/p/go-uuid/uuid/uuid.go",
        "src/code.google.com/p/go-uuid/uuid/version1.go",
        "src/code.google.com/p/go-uuid/uuid/version4.go",
    ],
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

go_library(
    name = "protoc-gen-go_generator",
    srcs = [
        "src/github.com/golang/protobuf/protoc-gen-go/generator/generator.go",
    ],
    package = "github.com/golang/protobuf/protoc-gen-go/generator",
    deps = [
        ":protobuf",
        ":protoc-gen-go_descriptor",
        ":protoc-gen-go_plugin",
    ],
)

go_library(
    name = "protoc-gen-go_plugin",
    srcs = [
        "src/github.com/golang/protobuf/protoc-gen-go/plugin/plugin.pb.go",
    ],
    package = "github.com/golang/protobuf/protoc-gen-go/plugin",
    deps = [
        ":protobuf",
        ":protoc-gen-go_descriptor",
    ],
)

go_library(
    name = "protoc-gen-go_descriptor",
    srcs = [
        "src/github.com/golang/protobuf/protoc-gen-go/descriptor/descriptor.pb.go",
    ],
    package = "github.com/golang/protobuf/protoc-gen-go/descriptor",
    deps = [
        ":protobuf",
    ],
)

go_library(
    name = "protoc-gen-go_internal_grpc",
    srcs = [
        "src/github.com/golang/protobuf/protoc-gen-go/internal/grpc/grpc.go",
    ],
    package = "github.com/golang/protobuf/protoc-gen-go/internal/grpc",
    deps = [
        ":protoc-gen-go_descriptor",
        ":protoc-gen-go_generator",
    ],
)

go_library(
    name = "protobuf",
    srcs = [
        "src/github.com/golang/protobuf/proto/clone.go",
        "src/github.com/golang/protobuf/proto/decode.go",
        "src/github.com/golang/protobuf/proto/encode.go",
        "src/github.com/golang/protobuf/proto/equal.go",
        "src/github.com/golang/protobuf/proto/extensions.go",
        "src/github.com/golang/protobuf/proto/lib.go",
        "src/github.com/golang/protobuf/proto/message_set.go",
        "src/github.com/golang/protobuf/proto/pointer_reflect.go",
        "src/github.com/golang/protobuf/proto/properties.go",
        "src/github.com/golang/protobuf/proto/text.go",
        "src/github.com/golang/protobuf/proto/text_parser.go",
    ],
    package = "github.com/golang/protobuf/proto",
)

go_library(
    name = "context",
    srcs = ["src/golang.org/x/net/context/context.go"],
    package = "golang.org/x/net/context",
)

go_library(
    name = "html",
    srcs = [
        "src/golang.org/x/net/html/const.go",
        "src/golang.org/x/net/html/doc.go",
        "src/golang.org/x/net/html/doctype.go",
        "src/golang.org/x/net/html/entity.go",
        "src/golang.org/x/net/html/escape.go",
        "src/golang.org/x/net/html/foreign.go",
        "src/golang.org/x/net/html/node.go",
        "src/golang.org/x/net/html/parse.go",
        "src/golang.org/x/net/html/render.go",
        "src/golang.org/x/net/html/token.go",
    ],
    package = "golang.org/x/net/html",
    deps = [
        ":html_atom",
    ],
)

go_library(
    name = "html_atom",
    srcs = [
        "src/golang.org/x/net/html/atom/atom.go",
        "src/golang.org/x/net/html/atom/table.go",
    ],
    package = "golang.org/x/net/html/atom",
)

go_library(
    name = "cloud",
    srcs = [
        "src/google.golang.org/cloud/cloud.go",
    ],
    package = "google.golang.org/cloud",
    deps = [
        ":cloud_internal",
        ":context",
    ],
)

go_library(
    name = "cloud_compute_metadata",
    srcs = [
        "src/google.golang.org/cloud/compute/metadata/go13.go",
        "src/google.golang.org/cloud/compute/metadata/metadata.go",
    ],
    package = "google.golang.org/cloud/compute/metadata",
    deps = [
        ":cloud_internal",
    ],
)

go_library(
    name = "cloud_storage",
    srcs = [
        "src/google.golang.org/cloud/storage/acl.go",
        "src/google.golang.org/cloud/storage/storage.go",
        "src/google.golang.org/cloud/storage/types.go",
    ],
    package = "google.golang.org/cloud/storage",
    deps = [
        ":api_googleapi",
        ":api_storage_v1",
        ":cloud_internal",
        ":context",
    ],
)

go_library(
    name = "cloud_internal",
    srcs = [
        "src/google.golang.org/cloud/internal/cloud.go",
    ],
    package = "google.golang.org/cloud/internal",
    deps = [
        ":context",
    ],
)

go_library(
    name = "api_storage_v1",
    srcs = [
        "src/google.golang.org/api/storage/v1/storage-gen.go",
    ],
    package = "google.golang.org/api/storage/v1",
    deps = [
        ":api_googleapi",
        ":context",
    ],
)

go_library(
    name = "api_googleapi",
    srcs = [
        "src/google.golang.org/api/googleapi/googleapi.go",
        "src/google.golang.org/api/googleapi/types.go",
    ],
    package = "google.golang.org/api/googleapi",
    deps = [
        ":api_googleapi_internal_uritemplates",
        ":context",
    ],
)

go_library(
    name = "api_googleapi_internal_uritemplates",
    srcs = [
        "src/google.golang.org/api/googleapi/internal/uritemplates/uritemplates.go",
        "src/google.golang.org/api/googleapi/internal/uritemplates/utils.go",
    ],
    package = "google.golang.org/api/googleapi/internal/uritemplates",
)

go_library(
    name = "grpc",
    srcs = [
        "src/google.golang.org/grpc/call.go",
        "src/google.golang.org/grpc/clientconn.go",
        "src/google.golang.org/grpc/doc.go",
        "src/google.golang.org/grpc/rpc_util.go",
        "src/google.golang.org/grpc/server.go",
        "src/google.golang.org/grpc/stream.go",
    ],
    package = "google.golang.org/grpc",
    deps = [
        ":context",
        ":grpc_codes",
        ":grpc_credentials",
        ":grpc_metadata",
        ":grpc_transport",
        ":protobuf",
    ],
)

go_library(
    name = "grpc_codes",
    srcs = [
        "src/google.golang.org/grpc/codes/codes.go",
    ],
    package = "google.golang.org/grpc/codes",
)

go_library(
    name = "grpc_metadata",
    srcs = [
        "src/google.golang.org/grpc/metadata/metadata.go",
    ],
    package = "google.golang.org/grpc/metadata",
    deps = [
        ":context",
    ],
)

go_library(
    name = "grpc_transport",
    srcs = [
        "src/google.golang.org/grpc/transport/control.go",
        "src/google.golang.org/grpc/transport/http2_client.go",
        "src/google.golang.org/grpc/transport/http2_server.go",
        "src/google.golang.org/grpc/transport/http_util.go",
        "src/google.golang.org/grpc/transport/transport.go",
    ],
    package = "google.golang.org/grpc/transport",
    deps = [
        ":context",
        ":grpc_codes",
        ":grpc_credentials",
        ":grpc_metadata",
        ":http2",
        ":http2_hpack",
    ],
)

go_library(
    name = "http2",
    srcs = [
        "src/github.com/bradfitz/http2/buffer.go",
        "src/github.com/bradfitz/http2/errors.go",
        "src/github.com/bradfitz/http2/flow.go",
        "src/github.com/bradfitz/http2/frame.go",
        "src/github.com/bradfitz/http2/gotrack.go",
        "src/github.com/bradfitz/http2/headermap.go",
        "src/github.com/bradfitz/http2/http2.go",
        "src/github.com/bradfitz/http2/pipe.go",
        "src/github.com/bradfitz/http2/server.go",
        "src/github.com/bradfitz/http2/transport.go",
        "src/github.com/bradfitz/http2/write.go",
        "src/github.com/bradfitz/http2/writesched.go",
    ],
    package = "github.com/bradfitz/http2",
    deps = [
        ":http2_hpack",
    ],
)

go_library(
    name = "http2_hpack",
    srcs = [
        "src/github.com/bradfitz/http2/hpack/encode.go",
        "src/github.com/bradfitz/http2/hpack/hpack.go",
        "src/github.com/bradfitz/http2/hpack/huffman.go",
        "src/github.com/bradfitz/http2/hpack/tables.go",
    ],
    package = "github.com/bradfitz/http2/hpack",
)

go_library(
    name = "grpc_credentials",
    srcs = [
        "src/google.golang.org/grpc/credentials/credentials.go",
    ],
    package = "google.golang.org/grpc/credentials",
    deps = [
        ":context",
        ":oauth2",
        ":oauth2_google",
        ":oauth2_jwt",
    ],
)

go_library(
    name = "oauth2",
    srcs = [
        "src/golang.org/x/oauth2/oauth2.go",
        "src/golang.org/x/oauth2/token.go",
        "src/golang.org/x/oauth2/transport.go",
    ],
    package = "golang.org/x/oauth2",
    deps = [
        ":context",
    ],
)

go_library(
    name = "oauth2_google",
    srcs = [
        "src/golang.org/x/oauth2/google/google.go",
        "src/golang.org/x/oauth2/google/sdk.go",
        "src/golang.org/x/oauth2/google/source_appengine.go",
    ],
    package = "golang.org/x/oauth2/google",
    deps = [
        ":cloud_compute_metadata",
        ":oauth2",
        ":oauth2_internal",
        ":oauth2_jwt",
    ],
)

go_library(
    name = "oauth2_jwt",
    srcs = [
        "src/golang.org/x/oauth2/jwt/jwt.go",
    ],
    package = "golang.org/x/oauth2/jwt",
    deps = [
        ":oauth2",
        ":oauth2_internal",
        ":oauth2_jws",
    ],
)

go_library(
    name = "oauth2_jws",
    srcs = [
        "src/golang.org/x/oauth2/jws/jws.go",
    ],
    package = "golang.org/x/oauth2/jws",
    deps = [],
)

go_library(
    name = "oauth2_internal",
    srcs = [
        "src/golang.org/x/oauth2/internal/oauth2.go",
    ],
    package = "golang.org/x/oauth2/internal",
)

go_library(
    name = "go_types",
    srcs = [
        "src/golang.org/x/tools/go/types/api.go",
        "src/golang.org/x/tools/go/types/assignments.go",
        "src/golang.org/x/tools/go/types/builtins.go",
        "src/golang.org/x/tools/go/types/call.go",
        "src/golang.org/x/tools/go/types/check.go",
        "src/golang.org/x/tools/go/types/conversions.go",
        "src/golang.org/x/tools/go/types/decl.go",
        "src/golang.org/x/tools/go/types/errors.go",
        "src/golang.org/x/tools/go/types/eval.go",
        "src/golang.org/x/tools/go/types/expr.go",
        "src/golang.org/x/tools/go/types/exprstring.go",
        "src/golang.org/x/tools/go/types/go11.go",
        "src/golang.org/x/tools/go/types/go12.go",
        "src/golang.org/x/tools/go/types/initorder.go",
        "src/golang.org/x/tools/go/types/labels.go",
        "src/golang.org/x/tools/go/types/lookup.go",
        "src/golang.org/x/tools/go/types/methodset.go",
        "src/golang.org/x/tools/go/types/methodsetcache.go",
        "src/golang.org/x/tools/go/types/object.go",
        "src/golang.org/x/tools/go/types/objset.go",
        "src/golang.org/x/tools/go/types/operand.go",
        "src/golang.org/x/tools/go/types/ordering.go",
        "src/golang.org/x/tools/go/types/package.go",
        "src/golang.org/x/tools/go/types/predicates.go",
        "src/golang.org/x/tools/go/types/resolver.go",
        "src/golang.org/x/tools/go/types/return.go",
        "src/golang.org/x/tools/go/types/scope.go",
        "src/golang.org/x/tools/go/types/selection.go",
        "src/golang.org/x/tools/go/types/sizes.go",
        "src/golang.org/x/tools/go/types/stmt.go",
        "src/golang.org/x/tools/go/types/type.go",
        "src/golang.org/x/tools/go/types/typestring.go",
        "src/golang.org/x/tools/go/types/typexpr.go",
        "src/golang.org/x/tools/go/types/universe.go",
    ],
    go_build = 1,
    package = "golang.org/x/tools/go/types",
)
