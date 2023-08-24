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
    features = [
        "-layering_check",  # TODO: protocolbuffers/protobuf#10889
    ],
    deps = [
        "//external:libmemcached",
        "//kythe/cxx/common:json_proto",
        "//kythe/cxx/common:lib",
        "//kythe/cxx/common:sha256_hasher",
        "//kythe/cxx/common/indexing:caching_output",
        "//kythe/proto:analysis_cc_proto",
        "//kythe/proto:common_cc_proto",
        "//kythe/proto:filecontext_cc_proto",
        "//kythe/proto:storage_cc_proto",
        "@com_google_absl//absl/memory",
        "@com_google_absl//absl/strings:str_format",
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
    features = [
        "-layering_check",  # TODO: protocolbuffers/protobuf#10889
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
        "@com_google_absl//absl/log",
        "@com_google_absl//absl/log:check",
        "@com_google_absl//absl/memory",
        "@com_google_absl//absl/status:statusor",
        "@com_google_absl//absl/strings:str_format",
        "@com_google_protobuf//:protobuf",
        "@com_google_protobuf//src/google/protobuf/io",
        "@com_google_protobuf//src/google/protobuf/io:gzip_stream",
        "@llvm-project//llvm:Support",
    ],
)

cc_library(
    name = "semantic_hash",
    srcs = ["semantic_hash.cc"],
    hdrs = ["semantic_hash.h"],
    deps = [
        "//third_party/llvm/src:clang_builtin_headers",
        "@com_google_absl//absl/log",
        "@com_google_absl//absl/log:check",
        "@llvm-project//clang:ast",
        "@llvm-project//llvm:Support",
    ],
)

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
        ":indexed_parent_map",
        "//third_party/llvm/src:clang_builtin_headers",
        "@com_google_absl//absl/log",
        "@llvm-project//clang:ast",
        "@llvm-project//clang:basic",
        "@llvm-project//clang:lex",
        "@llvm-project//llvm:Support",
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
        "@com_google_absl//absl/log",
        "@llvm-project//clang:ast",
        "@llvm-project//clang:basic",
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
        "@com_google_absl//absl/log",
        "@com_google_absl//absl/strings",
        "@com_googlesource_code_re2//:re2",
        "@llvm-project//clang:ast",
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
        "//kythe/cxx/common:sha256_hasher",
        "//third_party/llvm/src:clang_builtin_headers",
        "@com_google_absl//absl/container:flat_hash_set",
        "@com_google_absl//absl/log",
        "@com_google_absl//absl/log:check",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/strings:str_format",
        "@com_google_absl//absl/types:span",
        "@llvm-project//clang:basic",
        "@llvm-project//clang:lex",
        "@llvm-project//llvm:Support",
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
        ":decl_printer",
        ":graph_observer",
        ":indexed_parent_iterator",
        ":indexed_parent_map",
        ":indexer_library_support",
        ":kythe_claim_client",
        ":marked_source",
        ":node_set",
        ":recursive_type_visitor",
        ":semantic_hash",
        ":stream_adapter",
        ":type_map",
        "//kythe/cxx/common:lib",
        "//kythe/cxx/common:scope_guard",
        "//kythe/cxx/extractor:supported_language",
        "//third_party/llvm/src:clang_builtin_headers",
        "@com_google_absl//absl/container:flat_hash_set",
        "@com_google_absl//absl/flags:flag",
        "@com_google_absl//absl/log",
        "@com_google_absl//absl/log:check",
        "@com_google_absl//absl/log:die_if_null",
        "@com_google_absl//absl/memory",
        "@com_google_absl//absl/strings",
        "@com_googlesource_code_re2//:re2",
        "@llvm-project//clang:ast",
        "@llvm-project//clang:basic",
        "@llvm-project//clang:index",
        "@llvm-project//clang:lex",
        "@llvm-project//clang:sema",
        "@llvm-project//llvm:Support",
    ],
)

cc_library(
    name = "indexed_parent_map",
    srcs = ["indexed_parent_map.cc"],
    hdrs = ["indexed_parent_map.h"],
    deps = [
        "//kythe/cxx/common:scope_guard",
        "@com_google_absl//absl/base",
        "@com_google_absl//absl/functional:any_invocable",
        "@com_google_absl//absl/log",
        "@com_google_absl//absl/log:check",
        "@llvm-project//clang:ast",
        "@llvm-project//llvm:Support",
    ],
)

cc_library(
    name = "indexed_parent_iterator",
    srcs = ["indexed_parent_iterator.cc"],
    hdrs = ["indexed_parent_iterator.h"],
    deps = [
        ":indexed_parent_map",
        "@llvm-project//clang:ast",
    ],
)

cc_library(
    name = "type_map",
    srcs = ["type_map.cc"],
    hdrs = ["type_map.h"],
    deps = ["@llvm-project//clang:ast"],
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
        "@com_google_absl//absl/log",
        "@llvm-project//clang:ast",
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
        "@com_google_absl//absl/log",
        "@com_google_absl//absl/log:check",
        "@com_google_absl//absl/strings:str_format",
        "@llvm-project//clang:basic",
        "@llvm-project//clang:index",
        "@llvm-project//clang:lex",
        "@llvm-project//llvm:Support",
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
        "@com_google_absl//absl/container:flat_hash_set",
        "@com_google_absl//absl/flags:flag",
        "@com_google_absl//absl/log",
        "@com_google_absl//absl/log:check",
        "@com_google_absl//absl/log:die_if_null",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/strings:str_format",
        "@com_google_absl//absl/types:span",
        "@llvm-project//clang:ast",
        "@llvm-project//clang:basic",
        "@llvm-project//llvm:Support",
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
    features = [
        "-layering_check",  # TODO: protocolbuffers/protobuf#10889
    ],
    deps = [
        ":clang_range_finder",
        ":clang_utils",
        ":graph_observer",
        ":kythe_claim_client",
        "//kythe/cxx/common:scope_guard",
        "//third_party/llvm/src:clang_builtin_headers",
        "@com_google_absl//absl/flags:flag",
        "@com_google_absl//absl/log",
        "@com_google_absl//absl/log:check",
        "@com_google_protobuf//:protobuf",
        "@llvm-project//clang:ast",
        "@llvm-project//clang:basic",
        "@llvm-project//clang:format",
        "@llvm-project//clang:rewrite",
        "@llvm-project//clang:sema",
        "@llvm-project//llvm:Support",
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
    features = [
        "-layering_check",  # TODO: protocolbuffers/protobuf#10889
    ],
    deps = [
        ":clang_utils",
        ":indexer_ast_hooks",
        ":indexer_library_support",
        "//third_party/llvm/src:clang_builtin_headers",
        "@com_google_absl//absl/flags:flag",
        "@com_google_absl//absl/log",
        "@com_google_protobuf//:protobuf",
        "@com_google_protobuf//src/google/protobuf/io",
        "@com_google_protobuf//src/google/protobuf/io:tokenizer",
        "@llvm-project//clang:ast",
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
    features = [
        "-layering_check",  # TODO: protocolbuffers/protobuf#10889
    ],
    deps = [
        ":clang_utils",
        "//kythe/cxx/common:json_proto",
        "//kythe/cxx/common:lib",
        "//kythe/proto:analysis_cc_proto",
        "//kythe/proto:common_cc_proto",
        "//kythe/proto:storage_cc_proto",
        "@com_google_absl//absl/base:core_headers",
        "@com_google_absl//absl/container:flat_hash_map",
        "@com_google_absl//absl/log",
        "@com_google_absl//absl/log:check",
        "@com_google_absl//absl/memory",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/strings:str_format",
        "@com_google_protobuf//:protobuf",
        "@llvm-project//clang:basic",
        "@llvm-project//llvm:Support",
    ],
)

cc_test(
    name = "vfs_test",
    srcs = ["KytheVFS_test.cc"],
    deps = [
        ":vfs",
        "//kythe/proto:analysis_cc_proto",
        "//third_party:gtest",
        "//third_party:gtest_main",
        "@com_google_absl//absl/strings",
        "@llvm-project//llvm:Support",
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
        "@com_google_absl//absl/log",
        "@com_google_absl//absl/log:check",
        "@com_google_absl//absl/log:die_if_null",
        "@com_google_absl//absl/memory",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/strings:str_format",
        "@com_googlesource_code_re2//:re2",
        "@llvm-project//clang:basic",
        "@llvm-project//clang:frontend",
        "@llvm-project//clang:lex",
        "@llvm-project//clang:tooling",
        "@llvm-project//llvm:Support",
    ],
)

cc_binary(
    name = "indexer",
    srcs = [
        "KytheIndexerMain.cc",
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
        ":frontend",
        ":google_flags_library_support",
        ":imputed_constructor_library_support",
        ":indexer_ast_hooks",
        ":lib",
        ":proto_library_support",
        "//external:zlib",
        "//kythe/cxx/common:init",
        "//kythe/cxx/common:lib",
        "//kythe/cxx/common:re2_flag",
        "@com_google_absl//absl/flags:flag",
        "@com_google_absl//absl/flags:parse",
        "@com_google_absl//absl/flags:usage",
        "@com_google_absl//absl/log",
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
    features = [
        "-layering_check",  # TODO: protocolbuffers/protobuf#10889
    ],
    linkopts = select({
        "@platforms//os:osx": ["-headerpad_max_install_names"],
        "//conditions:default": [],
    }),
    deps = [
        ":indexer_ast_hooks",
        ":lib",
        "//kythe/cxx/common/indexing:output",
        "//kythe/cxx/common/indexing:testlib",
        "//third_party:gtest",
        "@com_github_inazarenko_protobuf_matchers//protobuf-matchers",
        "@com_google_protobuf//:protobuf",
        "@llvm-project//clang:ast",
        "@llvm-project//clang:frontend",
        "@llvm-project//clang:tooling",
        "@llvm-project//llvm:Support",
    ],
)

cc_library(
    name = "recursive_type_visitor",
    hdrs = ["recursive_type_visitor.h"],
    deps = [
        "@llvm-project//clang:ast",
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
        ":stream_adapter",
        "@com_google_absl//absl/log",
        "@com_google_absl//absl/log:check",
        "@com_google_absl//absl/log:die_if_null",
        "@llvm-project//clang:ast",
        "@llvm-project//clang:basic",
        "@llvm-project//clang:lex",
        "@llvm-project//llvm:Support",
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
        "@com_google_absl//absl/log:check",
        "@com_google_absl//absl/log:die_if_null",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/strings:str_format",
        "@llvm-project//clang:ast",
        "@llvm-project//clang:frontend",
        "@llvm-project//clang:lex",
        "@llvm-project//clang:tooling",
        "@llvm-project//llvm:Support",
    ],
)

cc_library(
    name = "stream_adapter",
    srcs = ["stream_adapter.cc"],
    hdrs = ["stream_adapter.h"],
    deps = [
        "@com_google_absl//absl/base:core_headers",
        "@com_google_absl//absl/functional:any_invocable",
        "@llvm-project//llvm:Support",
    ],
)

cc_library(
    name = "decl_printer",
    srcs = ["decl_printer.cc"],
    hdrs = ["decl_printer.h"],
    deps = [
        ":graph_observer",
        ":indexed_parent_iterator",
        ":indexed_parent_map",
        ":semantic_hash",
        "@com_google_absl//absl/base:core_headers",
        "@llvm-project//clang:ast",
        "@llvm-project//clang:basic",
        "@llvm-project//llvm:Support",
    ],
)

cc_test(
    name = "decl_printer_test",
    srcs = ["decl_printer_test.cc"],
    deps = [
        ":decl_printer",
        ":graph_observer",
        ":indexed_parent_map",
        ":semantic_hash",
        "//third_party:gtest",
        "//third_party:gtest_main",
        "@com_google_absl//absl/log:check",
        "@com_google_absl//absl/log:die_if_null",
        "@com_google_absl//absl/strings",
        "@llvm-project//clang:ast",
        "@llvm-project//clang:ast_matchers",
        "@llvm-project//clang:basic",
        "@llvm-project//clang:frontend",
        "@llvm-project//clang:tooling",
    ],
)
