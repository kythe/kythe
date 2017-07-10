package(default_visibility = ["//kythe:default_visibility"])

cc_library(
    name = "clang_utils",
    srcs = [
        "clang_utils.cc",
    ],
    hdrs = [
        "clang_utils.h",
    ],
    copts = [
        "-Wno-non-virtual-dtor",
        "-Wno-unused-variable",
        "-Wno-implicit-fallthrough",
    ],
    deps = [
        "//third_party/llvm",
        "//third_party/llvm/src:clang_builtin_headers",
        "@com_github_google_glog//:glog",
    ],
)

cc_library(
    name = "google_flags_library_support",
    srcs = [
        "GoogleFlagsLibrarySupport.cc",
    ],
    hdrs = [
        "GoogleFlagsLibrarySupport.h",
    ],
    copts = [
        "-Wno-non-virtual-dtor",
        "-Wno-unused-variable",
        "-Wno-implicit-fallthrough",
    ],
    deps = [
        ":indexer_library_support",
        ":indexer_ast_hooks",
        "//third_party/llvm",
        "//third_party/llvm/src:clang_builtin_headers",
        "@com_github_google_glog//:glog",
    ],
)

cc_library(
    name = "graph_observer",
    hdrs = [
        "GraphObserver.h",
    ],
    copts = [
        "-Wno-non-virtual-dtor",
        "-Wno-unused-variable",
        "-Wno-implicit-fallthrough",
    ],
    deps = [
        "//kythe/cxx/common/indexing:lib",
        "//third_party/llvm",
        "//third_party/llvm/src:clang_builtin_headers",
        "@com_github_google_glog//:glog",
    ],
)

cc_library(
    name = "indexer_ast_hooks",
    srcs = [
        "IndexerASTHooks.cc",
        "indexer_worklist.cc",
    ],
    hdrs = [
        "IndexerASTHooks.h",
        "indexer_worklist.h",
    ],
    copts = [
        "-Wno-non-virtual-dtor",
        "-Wno-unused-variable",
        "-Wno-implicit-fallthrough",
    ],
    deps = [
        ":marked_source",
        ":graph_observer",
        ":indexer_library_support",
        "//kythe/cxx/common/indexing:lib",
        "//kythe/cxx/common:lib",
        "//kythe/cxx/common:supported_language",
        "//third_party/llvm",
        "//third_party/llvm/src:clang_builtin_headers",
        "@com_github_google_glog//:glog",
    ],
)

cc_library(
    name = "indexer_library_support",
    hdrs = [
        "IndexerLibrarySupport.h",
    ],
    copts = [
        "-Wno-non-virtual-dtor",
        "-Wno-unused-variable",
        "-Wno-implicit-fallthrough",
    ],
    deps = [
        ":graph_observer",
        "//kythe/cxx/common/indexing:lib",
        "//kythe/cxx/common:lib",
        "//kythe/cxx/common:supported_language",
        "//third_party/llvm",
        "//third_party/llvm/src:clang_builtin_headers",
        "@com_github_google_glog//:glog",
    ],
)

cc_library(
    name = "indexer_pp_callbacks",
    srcs = [
        "IndexerPPCallbacks.cc",
    ],
    hdrs = [
        "IndexerPPCallbacks.h",
    ],
    copts = [
        "-Wno-non-virtual-dtor",
        "-Wno-unused-variable",
        "-Wno-implicit-fallthrough",
    ],
    deps = [
        ":graph_observer",
        ":indexer_ast_hooks",
        "//kythe/cxx/common:lib",
        "//third_party/llvm",
        "//third_party/llvm/src:clang_builtin_headers",
        "@com_github_google_glog//:glog",
    ],
)

cc_library(
    name = "kythe_graph_observer",
    srcs = [
        "KytheGraphObserver.cc",
    ],
    hdrs = [
        "KytheGraphObserver.h",
    ],
    copts = [
        "-Wno-non-virtual-dtor",
        "-Wno-unused-variable",
        "-Wno-implicit-fallthrough",
    ],
    deps = [
        ":graph_observer",
        ":indexer_ast_hooks",
        "//kythe/cxx/common/indexing:lib",
        "//kythe/cxx/common:lib",
        "//kythe/cxx/common:supported_language",
        "//third_party/llvm",
        "//third_party/llvm/src:clang_builtin_headers",
        "@com_github_google_glog//:glog",
    ],
)

cc_library(
    name = "marked_source",
    srcs = [
        "marked_source.cc",
    ],
    hdrs = [
        "marked_source.h",
    ],
    copts = [
        "-Wno-non-virtual-dtor",
        "-Wno-unused-variable",
        "-Wno-implicit-fallthrough",
    ],
    deps = [
        ":clang_utils",
        ":graph_observer",
        "//kythe/cxx/common/indexing:lib",
        "//third_party/llvm",
        "//third_party/llvm/src:clang_builtin_headers",
        "@com_github_google_glog//:glog",
    ],
)

cc_library(
    name = "proto_library_support",
    srcs = [
        "ProtoLibrarySupport.cc",
    ],
    hdrs = [
        "ProtoLibrarySupport.h",
    ],
    copts = [
        "-Wno-non-virtual-dtor",
        "-Wno-unused-variable",
        "-Wno-implicit-fallthrough",
    ],
    deps = [
        ":indexer_ast_hooks",
        ":indexer_library_support",
        "//third_party/llvm",
        "//third_party/llvm/src:clang_builtin_headers",
        "//third_party/proto:protobuf",
        "@com_github_google_glog//:glog",
    ],
)

cc_library(
    name = "lib",
    srcs = [
        "IndexerFrontendAction.cc",
    ],
    hdrs = [
        "IndexerFrontendAction.h",
    ],
    copts = [
        "-Wno-non-virtual-dtor",
        "-Wno-unused-variable",
        "-Wno-implicit-fallthrough",
    ],
    deps = [
        ":clang_utils",
        ":google_flags_library_support",
        ":graph_observer",
        ":indexer_pp_callbacks",
        ":kythe_graph_observer",
        ":marked_source",
        ":proto_library_support",
        "//external:libmemcached",
        "//kythe/cxx/common/indexing:lib",
        "//kythe/cxx/common:index_pack",
        "//kythe/cxx/common:json_proto",
        "//kythe/cxx/common:lib",
        "//kythe/cxx/common:supported_language",
        "//kythe/proto:analysis_proto_cc",
        "//kythe/proto:common_proto_cc",
        "//kythe/proto:cxx_proto_cc",
        "//kythe/proto:storage_proto_cc",
        "//third_party/llvm",
        "//third_party/llvm/src:clang_builtin_headers",
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
