load("//tools/cpp:cc_release_binary.bzl", "cc_release_binary")

package(
    default_visibility = ["//kythe:default_visibility"],
    features = ["layering_check"],
)

cc_library(
    name = "kythe_claim_client",
    srcs = [
        "DynamicClaimClient.cc",
        "KytheClaimClient.cc",
    ],
    hdrs = [
        "DynamicClaimClient.h",
        "KytheClaimClient.h",
    ],
    copts = [
        "-Wno-non-virtual-dtor",
        "-Wno-unused-variable",
        "-Wno-implicit-fallthrough",
    ],
    deps = [
        "//external:libmemcached",
        "//kythe/cxx/common:json_proto",
        "//kythe/cxx/common:lib",
        "//kythe/cxx/common/indexing:caching_output",
        "//kythe/proto:analysis_cc_proto",
        "//kythe/proto:common_cc_proto",
        "//kythe/proto:filecontext_cc_proto",
        "//kythe/proto:storage_cc_proto",
        "@boringssl//:crypto",
        "@com_google_absl//absl/memory",
        "@com_google_absl//absl/strings:str_format",
        "@com_google_absl//absl/types:optional",
        "@com_google_protobuf//:protobuf",
    ],
)

cc_library(
    name = "frontend",
    srcs = [
        "frontend.cc",
    ],
    hdrs = [
        "frontend.h",
    ],
    copts = [
        "-Wno-non-virtual-dtor",
        "-Wno-unused-variable",
        "-Wno-implicit-fallthrough",
    ],
    deps = [
        ":clang_utils",
        ":kythe_claim_client",
        "//kythe/cxx/common:kzip_reader",
        "//kythe/cxx/common:path_utils",
        "//kythe/cxx/common/indexing:caching_output",
        "//kythe/proto:buildinfo_cc_proto",
        "//kythe/proto:claim_cc_proto",
        "@com_google_absl//absl/flags:flag",
        "@com_google_absl//absl/memory",
        "@com_google_absl//absl/strings:str_format",
        "@com_google_protobuf//:protobuf",
        "@org_llvm//:LLVMSupport",
    ],
)

cc_library(
    name = "semantic_hash",
    srcs = ["semantic_hash.cc"],
    hdrs = ["semantic_hash.h"],
    deps = [
        "//third_party/llvm/src:clang_builtin_headers",
        "@com_github_google_glog//:glog",
        "@org_llvm//:LLVMSupport",
        "@org_llvm//:clangAST",
    ],
)

cc_library(
    name = "clang_utils",
    srcs = [
        "clang_utils.cc",
    ],
    hdrs = [
        "clang_utils.h",
        "proto_conversions.h",
    ],
    copts = [
        "-Wno-non-virtual-dtor",
        "-Wno-unused-variable",
        "-Wno-implicit-fallthrough",
    ],
    deps = [
        ":indexed_parent_map",
        "//third_party/llvm/src:clang_builtin_headers",
        "@com_github_google_glog//:glog",
        "@org_llvm//:LLVMSupport",
        "@org_llvm//:clangAST",
        "@org_llvm//:clangBasic",
        "@org_llvm//:clangLex",
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
        ":indexer_ast_hooks",
        ":indexer_library_support",
        "//third_party/llvm/src:clang_builtin_headers",
        "@com_github_google_glog//:glog",
        "@org_llvm//:clangAST",
        "@org_llvm//:clangBasic",
    ],
)

cc_library(
    name = "imputed_constructor_library_support",
    srcs = [
        "ImputedConstructorSupport.cc",
    ],
    hdrs = [
        "ImputedConstructorSupport.h",
    ],
    deps = [
        ":indexer_ast_hooks",
        ":indexer_library_support",
        "//third_party/llvm/src:clang_builtin_headers",
        "@com_github_google_glog//:glog",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/types:optional",
        "@com_googlesource_code_re2//:re2",
        "@org_llvm//:clangAST",
    ],
)

cc_library(
    name = "graph_observer",
    srcs = [
        "GraphObserver.cc",
    ],
    hdrs = [
        "GraphObserver.h",
    ],
    copts = [
        "-Wno-non-virtual-dtor",
        "-Wno-unused-variable",
        "-Wno-implicit-fallthrough",
    ],
    deps = [
        ":kythe_claim_client",
        "//third_party/llvm/src:clang_builtin_headers",
        "@boringssl//:crypto",
        "@com_github_google_glog//:glog",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/types:optional",
        "@com_google_absl//absl/types:span",
        "@org_llvm//:LLVMSupport",
        "@org_llvm//:clangBasic",
        "@org_llvm//:clangLex",
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
        ":clang_range_finder",
        ":clang_utils",
        ":graph_observer",
        ":indexed_parent_iterator",
        ":indexed_parent_map",
        ":indexer_library_support",
        ":kythe_claim_client",
        ":marked_source",
        ":node_set",
        ":recursive_type_visitor",
        ":semantic_hash",
        ":type_map",
        "//kythe/cxx/common:lib",
        "//kythe/cxx/common:scope_guard",
        "//kythe/cxx/extractor:supported_language",
        "//third_party/llvm/src:clang_builtin_headers",
        "@com_github_google_glog//:glog",
        "@com_google_absl//absl/container:flat_hash_set",
        "@com_google_absl//absl/flags:flag",
        "@com_google_absl//absl/memory",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/types:optional",
        "@org_llvm//:LLVMSupport",
        "@org_llvm//:clangAST",
        "@org_llvm//:clangBasic",
        "@org_llvm//:clangIndex",
        "@org_llvm//:clangLex",
        "@org_llvm//:clangSema",
    ],
)

cc_library(
    name = "indexed_parent_map",
    srcs = ["indexed_parent_map.cc"],
    hdrs = ["indexed_parent_map.h"],
    deps = [
        "//kythe/cxx/common:scope_guard",
        "@com_github_google_glog//:glog",
        "@org_llvm//:LLVMSupport",
        "@org_llvm//:clangAST",
    ],
)

cc_library(
    name = "indexed_parent_iterator",
    srcs = ["indexed_parent_iterator.cc"],
    hdrs = ["indexed_parent_iterator.h"],
    deps = [
        ":indexed_parent_map",
        "@com_google_absl//absl/types:optional",
        "@org_llvm//:clangAST",
    ],
)

cc_library(
    name = "type_map",
    srcs = ["type_map.cc"],
    hdrs = ["type_map.h"],
    deps = ["@org_llvm//:clangAST"],
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
        ":kythe_claim_client",
        "//kythe/cxx/common:lib",
        "//kythe/cxx/extractor:supported_language",
        "//third_party/llvm/src:clang_builtin_headers",
        "@com_github_google_glog//:glog",
        "@org_llvm//:clangAST",
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
        "//kythe/cxx/extractor:path_utils",
        "//third_party/llvm/src:clang_builtin_headers",
        "@com_github_google_glog//:glog",
        "@com_google_absl//absl/strings:str_format",
        "@org_llvm//:LLVMSupport",
        "@org_llvm//:clangBasic",
        "@org_llvm//:clangIndex",
        "@org_llvm//:clangLex",
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
        ":kythe_claim_client",
        ":vfs",
        "//kythe/cxx/common:lib",
        "//kythe/cxx/common:path_utils",
        "//kythe/cxx/common/indexing:output",
        "//kythe/cxx/common/schema:edges",
        "//kythe/cxx/extractor:supported_language",
        "//third_party/llvm/src:clang_builtin_headers",
        "@com_github_google_glog//:glog",
        "@com_google_absl//absl/container:flat_hash_set",
        "@com_google_absl//absl/flags:flag",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/strings:str_format",
        "@com_google_absl//absl/types:span",
        "@org_llvm//:LLVMSupport",
        "@org_llvm//:clangAST",
        "@org_llvm//:clangBasic",
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
        ":clang_range_finder",
        ":clang_utils",
        ":graph_observer",
        ":kythe_claim_client",
        "//third_party/llvm/src:clang_builtin_headers",
        "@com_github_google_glog//:glog",
        "@com_google_absl//absl/flags:flag",
        "@com_google_protobuf//:protobuf",
        "@org_llvm//:LLVMSupport",
        "@org_llvm//:clangAST",
        "@org_llvm//:clangBasic",
        "@org_llvm//:clangFormat",
        "@org_llvm//:clangRewrite",
        "@org_llvm//:clangSema",
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
        ":clang_utils",
        ":indexer_ast_hooks",
        ":indexer_library_support",
        "//third_party/llvm/src:clang_builtin_headers",
        "@com_github_google_glog//:glog",
        "@com_google_absl//absl/flags:flag",
        "@com_google_protobuf//:protobuf",
        "@org_llvm//:clangAST",
    ],
)

cc_library(
    name = "vfs",
    srcs = [
        "KytheVFS.cc",
    ],
    hdrs = [
        "KytheVFS.h",
    ],
    copts = [
        "-Wno-non-virtual-dtor",
        "-Wno-unused-variable",
        "-Wno-implicit-fallthrough",
    ],
    deps = [
        ":clang_utils",
        "//kythe/cxx/common:json_proto",
        "//kythe/cxx/common:lib",
        "//kythe/proto:analysis_cc_proto",
        "//kythe/proto:common_cc_proto",
        "//kythe/proto:storage_cc_proto",
        "@com_google_absl//absl/memory",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/strings:str_format",
        "@com_google_absl//absl/types:optional",
        "@com_google_protobuf//:protobuf",
        "@org_llvm//:LLVMSupport",
        "@org_llvm//:clangBasic",
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
        ":imputed_constructor_library_support",
        ":indexer_pp_callbacks",
        ":kythe_claim_client",
        ":kythe_graph_observer",
        ":marked_source",
        ":proto_library_support",
        ":vfs",
        "//external:libmemcached",
        "//kythe/cxx/common:json_proto",
        "//kythe/cxx/common:lib",
        "//kythe/cxx/common/indexing:output",
        "//kythe/cxx/extractor:cxx_details",
        "//kythe/cxx/extractor:supported_language",
        "//kythe/proto:analysis_cc_proto",
        "//kythe/proto:buildinfo_cc_proto",
        "//kythe/proto:common_cc_proto",
        "//kythe/proto:cxx_cc_proto",
        "//kythe/proto:filecontext_cc_proto",
        "//kythe/proto:storage_cc_proto",
        "//third_party/llvm/src:clang_builtin_headers",
        "@com_github_google_glog//:glog",
        "@com_google_absl//absl/memory",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/strings:str_format",
        "@org_llvm//:LLVMSupport",
        "@org_llvm//:clangFrontend",
        "@org_llvm//:clangLex",
        "@org_llvm//:clangTooling",
    ],
)

cc_release_binary(
    name = "indexer",
    srcs = [
        "KytheIndexerMain.cc",
    ],
    copts = [
        "-Wno-non-virtual-dtor",
        "-Wno-unused-variable",
        "-Wno-implicit-fallthrough",
    ],
    deps = [
        ":frontend",
        ":google_flags_library_support",
        ":imputed_constructor_library_support",
        ":indexer_ast_hooks",
        ":lib",
        ":proto_library_support",
        "//external:zlib",
        "//kythe/cxx/common:lib",
        "@com_github_google_glog//:glog",
        "@com_google_absl//absl/flags:flag",
        "@com_google_absl//absl/flags:parse",
        "@com_google_absl//absl/flags:usage",
        "@com_google_absl//absl/memory",
        "@com_google_absl//absl/strings:str_format",
        "@com_google_protobuf//:protobuf",
    ],
)

cc_library(
    name = "node_set",
    hdrs = ["node_set.h"],
    deps = [
        ":graph_observer",
        "@com_google_absl//absl/types:optional",
    ],
)

cc_test(
    name = "test",
    size = "small",
    srcs = [
        "KytheIndexerUnitTest.cc",
    ],
    copts = [
        "-Wno-non-virtual-dtor",
        "-Wno-unused-variable",
        "-Wno-implicit-fallthrough",
    ],
    linkopts = select({
        ":darwin": ["-headerpad_max_install_names"],
        "//conditions:default": [],
    }),
    deps = [
        ":indexer_ast_hooks",
        ":lib",
        "//kythe/cxx/common/indexing:output",
        "//kythe/cxx/common/indexing:testlib",
        "//third_party:gtest",
        "@com_google_absl//absl/memory",
        "@com_google_protobuf//:protobuf",
        "@org_llvm//:LLVMSupport",
        "@org_llvm//:clangAST",
        "@org_llvm//:clangFrontend",
        "@org_llvm//:clangTooling",
    ],
)

cc_library(
    name = "recursive_type_visitor",
    hdrs = ["recursive_type_visitor.h"],
    deps = [
        "@org_llvm//:clangAST",
    ],
)

cc_test(
    name = "recursive_type_visitor_test",
    srcs = ["recursive_type_visitor_test.cc"],
    deps = [
        ":recursive_type_visitor",
        "//third_party:gtest",
        "//third_party:gtest_main",
    ],
)

cc_library(
    name = "clang_range_finder",
    srcs = ["clang_range_finder.cc"],
    hdrs = ["clang_range_finder.h"],
    deps = [
        "@com_github_google_glog//:glog",
        "@org_llvm//:LLVMSupport",
        "@org_llvm//:clangAST",
        "@org_llvm//:clangBasic",
        "@org_llvm//:clangLex",
    ],
)

cc_test(
    name = "clang_range_finder_test",
    srcs = [
        "clang_range_finder_test.cc",
    ],
    deps = [
        ":clang_range_finder",
        "//third_party:gtest",
        "//third_party:gtest_main",
        "@com_github_google_glog//:glog",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/strings:str_format",
        "@org_llvm//:LLVMSupport",
        "@org_llvm//:clangAST",
        "@org_llvm//:clangFrontend",
        "@org_llvm//:clangLex",
        "@org_llvm//:clangTooling",
    ],
)

config_setting(
    name = "darwin",
    values = {"cpu": "darwin"},
    visibility = ["//visibility:public"],
)
