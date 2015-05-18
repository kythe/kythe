package(default_visibility = ["//kythe:default_visibility"])

cc_library(
    name = "lib",
    srcs = [
        "IndexerASTHooks.cc",
        "IndexerFrontendAction.cc",
        "IndexerLibrarySupport.cc",
        "IndexerPPCallbacks.cc",
        "KytheClaimClient.cc",
        "KytheGraphObserver.cc",
        "KytheGraphRecorder.cc",
        "KytheVFS.cc",
    ],
    hdrs = [
        "GraphObserver.h",
        "IndexerASTHooks.h",
        "IndexerFrontendAction.h",
        "IndexerLibrarySupport.h",
        "IndexerPPCallbacks.h",
        "KytheClaimClient.h",
        "KytheGraphObserver.h",
        "KytheGraphRecorder.h",
        "KytheOutputStream.h",
        "KytheVFS.h",
    ],
    copts = [
        "-Wno-non-virtual-dtor",
        "-Wno-unused-variable",
        "-Wno-implicit-fallthrough",
    ],
    deps = [
        "//kythe/cxx/common:json_proto",
        "//kythe/cxx/common:lib",
        "//kythe/proto:cxx_proto_cc",
        "//kythe/proto:storage_proto_cc",
        "//third_party/googleflags:gflags",
        "//third_party/llvm",
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
        "//kythe/cxx/common:lib",
        "//kythe/proto:analysis_proto_cc",
        "//kythe/proto:claim_proto_cc",
        "//kythe/proto:cxx_proto_cc",
        "//kythe/proto:storage_proto_cc",
        "//third_party/googleflags:gflags",
        "//third_party/googlelog:glog",
        "//third_party/llvm",
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
    hdrs = [
        "RecordingOutputStream.h",
    ],
    copts = [
        "-Wno-non-virtual-dtor",
        "-Wno-unused-variable",
        "-Wno-implicit-fallthrough",
    ],
    deps = [
        ":lib",
        "//kythe/proto:cxx_proto_cc",
        "//kythe/proto:storage_proto_cc",
        "//third_party/googletest",
        "//third_party/llvm",
        "//third_party/proto:protobuf",
    ],
)

cc_test(
    name = "test",
    deps = [
        ":testlib",
    ],
)
