package(default_visibility = ["//kythe:default_visibility"])

cc_library(
    name = "lib",
    srcs = [
        "KytheClaimClient.cc",
        "KytheGraphRecorder.cc",
        "KytheOutputStream.cc",
        "KytheVFS.cc",
    ],
    hdrs = [
        "KytheClaimClient.h",
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
        "//kythe/cxx/common:index_pack",
        "//kythe/cxx/common:json_proto",
        "//kythe/cxx/common:lib",
        "//kythe/proto:analysis_proto_cc",
        "//kythe/proto:storage_proto_cc",
        "//third_party:libmemcached",
        "//third_party/googleflags:gflags",
        "//third_party/proto:protobuf",
    ],
)

cc_library(
    name = "testlib",
    hdrs = [
        "RecordingOutputStream.h"
    ],
    deps = [
        ":lib",
    ],
)
