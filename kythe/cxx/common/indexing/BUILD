package(default_visibility = ["//kythe:default_visibility"])

cc_library(
    name = "output",
    srcs = [
        "KytheGraphRecorder.cc",
    ],
    hdrs = [
        "KytheGraphRecorder.h",
        "KytheOutputStream.h",
    ],
    deps = [
        "//kythe/proto:analysis_cc_proto",
        "//kythe/proto:common_cc_proto",
        "//kythe/proto:storage_cc_proto",
        "@com_github_gflags_gflags//:gflags",
        "@com_google_absl//absl/memory",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/types:optional",
        "@com_google_protobuf//:protobuf",
    ],
)

cc_library(
    name = "caching_output",
    srcs = [
        "KytheCachingOutput.cc",
    ],
    hdrs = [
        "KytheCachingOutput.h",
    ],
    deps = [
        ":output",
        "//external:libmemcached",
        "//kythe/proto:analysis_cc_proto",
        "//kythe/proto:common_cc_proto",
        "//kythe/proto:storage_cc_proto",
        "//third_party/llvm",
        "@com_github_gflags_gflags//:gflags",
        "@com_google_absl//absl/memory",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/types:optional",
        "@com_google_protobuf//:protobuf",
    ],
)

cc_library(
    name = "lib",
    srcs = [
        "KytheClaimClient.cc",
        "KytheVFS.cc",
    ],
    hdrs = [
        "KytheClaimClient.h",
        "KytheVFS.h",
    ],
    copts = [
        "-Wno-non-virtual-dtor",
        "-Wno-unused-variable",
        "-Wno-implicit-fallthrough",
    ],
    deps = [
        ":caching_output",
        "//external:libmemcached",
        "//kythe/cxx/common:index_pack",
        "//kythe/cxx/common:json_proto",
        "//kythe/cxx/common:lib",
        "//kythe/proto:analysis_cc_proto",
        "//kythe/proto:common_cc_proto",
        "//kythe/proto:storage_cc_proto",
        "@com_github_gflags_gflags//:gflags",
        "@com_google_absl//absl/memory",
        "@com_google_absl//absl/types:optional",
        "@com_google_protobuf//:protobuf",
    ],
)

cc_library(
    name = "testlib",
    hdrs = [
        "RecordingOutputStream.h",
    ],
    deps = [
        ":lib",
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
        ":lib",
        "//kythe/proto:claim_cc_proto",
        "@com_google_absl//absl/memory",
    ],
)
