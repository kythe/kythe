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
        "//external:proto/protobuf",
        "//kythe/cxx/common:index_pack",
        "//kythe/cxx/common:json_proto",
        "//kythe/cxx/common:lib",
        "//kythe/cxx/common/indexing:lib",
        "//kythe/proto:analysis_proto_cc",
        "//kythe/proto:cxx_proto_cc",
        "//kythe/proto:storage_proto_cc",
        "//third_party/llvm",
        "//third_party/llvm/src:clang_builtin_headers",
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
        "//external:glog",
        "//external:proto/protobuf",
        "//external:zlib",
        "//kythe/cxx/common:lib",
        "//kythe/cxx/common/indexing:frontend",
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
        "//external:proto/protobuf",
        "//kythe/cxx/common/indexing:testlib",
        "//third_party/llvm",
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
