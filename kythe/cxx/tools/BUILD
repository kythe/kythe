package(
    default_visibility = ["//kythe:default_visibility"],
    features = ["layering_check"],
)

cc_binary(
    name = "static_claim",
    srcs = [
        "static_claim_main.cc",
    ],
    copts = [
        "-Wno-non-virtual-dtor",
        "-Wno-unused-variable",
        "-Wno-implicit-fallthrough",
    ],
    features = [
        "-layering_check",  # TODO: protocolbuffers/protobuf#10889
    ],
    deps = [
        "//external:zlib",
        "//kythe/cxx/common:init",
        "//kythe/cxx/common:kzip_reader",
        "//kythe/cxx/common:lib",
        "//kythe/cxx/common:re2_flag",
        "//kythe/proto:analysis_cc_proto",
        "//kythe/proto:claim_cc_proto",
        "//kythe/proto:filecontext_cc_proto",
        "@com_google_absl//absl/flags:flag",
        "@com_google_absl//absl/flags:parse",
        "@com_google_absl//absl/flags:usage",
        "@com_google_absl//absl/log",
        "@com_google_absl//absl/log:check",
        "@com_google_absl//absl/strings:str_format",
        "@com_google_protobuf//:protobuf",
        "@com_google_protobuf//src/google/protobuf/io",
        "@com_google_protobuf//src/google/protobuf/io:gzip_stream",
    ],
)

cc_binary(
    name = "proto_metadata_plugin",
    srcs = ["proto_metadata_plugin.cc"],
    features = [
        "-layering_check",  # TODO: protocolbuffers/protobuf#10889
    ],
    deps = [
        "//kythe/cxx/common:init",
        "@com_google_absl//absl/container:flat_hash_map",
        "@com_google_absl//absl/container:node_hash_map",
        "@com_google_absl//absl/log",
        "@com_google_absl//absl/memory",
        "@com_google_absl//absl/strings",
        "@com_google_protobuf//:protobuf",
        "@com_google_protobuf//:protoc_lib",
        "@com_google_protobuf//src/google/protobuf/compiler:code_generator",
        "@com_google_protobuf//src/google/protobuf/compiler/cpp",
        "@com_google_protobuf//src/google/protobuf/io",
        "@com_google_protobuf//src/google/protobuf/io:gzip_stream",
        "@com_google_protobuf//src/google/protobuf/io:printer",
    ],
)
