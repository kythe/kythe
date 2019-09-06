package(default_visibility = ["//kythe:default_visibility"])

load("//:visibility.bzl", "PUBLIC_VISIBILITY")

cc_binary(
    name = "kindex_tool",
    testonly = 1,
    srcs = [
        "kindex_tool_main.cc",
    ],
    copts = [
        "-Wno-non-virtual-dtor",
        "-Wno-unused-variable",
        "-Wno-implicit-fallthrough",
    ],
    visibility = [
        "//kythe/cxx/extractor/testdata:__pkg__",
        "//kythe/cxx/indexer/cxx/testdata:__pkg__",
        "//kythe/cxx/tools/testdata:__pkg__",
    ],
    deps = [
        "//external:zlib",
        "//kythe/cxx/common:kzip_reader",
        "//kythe/proto:analysis_cc_proto",
        "//kythe/proto:buildinfo_cc_proto",
        "//kythe/proto:cxx_cc_proto",
        "//kythe/proto:filecontext_cc_proto",
        "@com_github_google_glog//:glog",
        "@com_google_absl//absl/container:flat_hash_map",
        "@com_google_absl//absl/flags:flag",
        "@com_google_absl//absl/flags:parse",
        "@com_google_absl//absl/flags:usage",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/strings:str_format",
        "@com_google_protobuf//:protobuf",
        "@com_googlesource_code_re2//:re2",
    ],
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
    deps = [
        "//external:zlib",
        "//kythe/cxx/common:lib",
        "//kythe/cxx/extractor:index_pack",
        "//kythe/proto:analysis_cc_proto",
        "//kythe/proto:claim_cc_proto",
        "//kythe/proto:filecontext_cc_proto",
        "@com_github_google_glog//:glog",
        "@com_google_absl//absl/flags:flag",
        "@com_google_absl//absl/flags:parse",
        "@com_google_absl//absl/flags:usage",
        "@com_google_absl//absl/strings:str_format",
        "@com_google_protobuf//:protobuf",
    ],
)

cc_binary(
    name = "shuck",
    srcs = [
        "shuck_main.cc",
    ],
    copts = [
        "-Wno-non-virtual-dtor",
        "-Wno-unused-variable",
        "-Wno-implicit-fallthrough",
    ],
    deps = [
        "//external:zlib",
        "//kythe/cxx/extractor:index_pack",
        "//kythe/proto:analysis_cc_proto",
        "//kythe/proto:claim_cc_proto",
        "@com_github_google_glog//:glog",
        "@com_google_absl//absl/flags:flag",
        "@com_google_absl//absl/flags:parse",
        "@com_google_absl//absl/strings:str_format",
        "@com_google_protobuf//:protobuf",
    ],
)

cc_binary(
    name = "proto_metadata_plugin",
    srcs = ["proto_metadata_plugin.cc"],
    deps = [
        "@com_github_google_glog//:glog",
        "@com_google_absl//absl/container:flat_hash_map",
        "@com_google_absl//absl/container:node_hash_map",
        "@com_google_absl//absl/memory",
        "@com_google_absl//absl/strings",
        "@com_google_protobuf//:protobuf",
        "@com_google_protobuf//:protoc_lib",
    ],
)
