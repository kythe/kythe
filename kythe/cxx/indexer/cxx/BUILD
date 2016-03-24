package(default_visibility = ["//kythe:default_visibility"])

cc_library(
    name = "lib",
    srcs = [
        "IndexerASTHooks.cc",
        "IndexerFrontendAction.cc",
        "IndexerLibrarySupport.cc",
        "IndexerPPCallbacks.cc",
        "KytheGraphObserver.cc",
    ],
    hdrs = [
        "GraphObserver.h",
        "IndexerASTHooks.h",
        "IndexerFrontendAction.h",
        "IndexerLibrarySupport.h",
        "IndexerPPCallbacks.h",
        "KytheGraphObserver.h",
    ],
    copts = [
        "-Wno-non-virtual-dtor",
        "-Wno-unused-variable",
        "-Wno-implicit-fallthrough",
    ],
    deps = [
        "//external:gflags",
        "//external:libmemcached",
        "//kythe/cxx/common:index_pack",
        "//kythe/cxx/common:json_proto",
        "//kythe/cxx/common:lib",
        "//kythe/cxx/common/indexing:lib",
        "//kythe/proto:analysis_proto_cc",
        "//kythe/proto:cxx_proto_cc",
        "//kythe/proto:storage_proto_cc",
        "//third_party/llvm",
        "//third_party/llvm/src:clang_builtin_headers",
        "//third_party/proto:protobuf",
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
        "//external:gflags",
        "//kythe/cxx/common:lib",
        "//kythe/cxx/common/indexing:frontend",
        "//external:glog",
        "//third_party/proto:protobuf",
        "//third_party/zlib",
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
        "//external:googletest",
        "//kythe/cxx/common/indexing:testlib",
        "//kythe/proto:cxx_proto_cc",
        "//kythe/proto:storage_proto_cc",
        "//third_party/llvm",
        "//third_party/proto:protobuf",
    ],
)

cc_test(
    name = "test",
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
