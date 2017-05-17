package(default_visibility = ["//kythe:default_visibility"])

cc_library(
    name = "lib",
    srcs = [
        "IndexerASTHooks.cc",
        "IndexerFrontendAction.cc",
        "IndexerLibrarySupport.cc",
        "IndexerPPCallbacks.cc",
        "KytheGraphObserver.cc",
        "ProtoLibrarySupport.cc",
        "clang_utils.cc",
        "indexer_worklist.cc",
        "marked_source.cc",
    ],
    hdrs = [
        "GraphObserver.h",
        "IndexerASTHooks.h",
        "IndexerFrontendAction.h",
        "IndexerLibrarySupport.h",
        "IndexerPPCallbacks.h",
        "KytheGraphObserver.h",
        "ProtoLibrarySupport.h",
        "clang_utils.h",
        "indexer_worklist.h",
        "marked_source.h",
    ],
    copts = [
        "-Wno-non-virtual-dtor",
        "-Wno-unused-variable",
        "-Wno-implicit-fallthrough",
    ],
    deps = [
        "//external:libmemcached",
        "//kythe/cxx/common:index_pack",
        "//kythe/cxx/common:json_proto",
        "//kythe/cxx/common:lib",
        "//kythe/cxx/common:supported_language",
        "//kythe/cxx/common/indexing:lib",
        "//kythe/proto:analysis_proto_cc",
        "//kythe/proto:common_proto_cc",
        "//kythe/proto:cxx_proto_cc",
        "//kythe/proto:storage_proto_cc",
        "//third_party/llvm",
        "//third_party/llvm/src:clang_builtin_headers",
        "//third_party/proto:protobuf",
        "@com_github_gflags_gflags//:gflags",
        "@com_github_google_glog//:glog",
    ],
)

cc_library(
    name = "cmdlib",
    srcs = [
        "KytheIndexerMain.cc",
    ],
    copts = [
        "-Wno-non-virtual-dtor",
        "-Wno-unused-variable",
        "-Wno-implicit-fallthrough",
    ],
    deps = [
        ":lib",
        "//kythe/cxx/common:lib",
        "//kythe/cxx/common/indexing:frontend",
        "//third_party/proto:protobuf",
        "//third_party/zlib",
        "@com_github_gflags_gflags//:gflags",
        "@com_github_google_glog//:glog",
    ],
)

cc_binary(
    name = "indexer",
    deps = [
        ":cmdlib",
    ],
)

cc_library(
    name = "testlib",
    testonly = 1,
    srcs = [
        "KytheIndexerUnitTest.cc",
    ],
    copts = [
        "-Wno-non-virtual-dtor",
        "-Wno-unused-variable",
        "-Wno-implicit-fallthrough",
    ],
    deps = [
        ":lib",
        "//kythe/cxx/common/indexing:testlib",
        "//third_party:gtest",
        "//third_party/llvm",
        "//third_party/proto:protobuf",
    ],
)

cc_test(
    name = "test",
    size = "small",
    linkopts = select({
        ":darwin": ["-headerpad_max_install_names"],
        "//conditions:default": [],
    }),
    deps = [
        ":testlib",
    ],
)

config_setting(
    name = "darwin",
    values = {"cpu": "darwin"},
    visibility = ["//visibility:public"],
)
