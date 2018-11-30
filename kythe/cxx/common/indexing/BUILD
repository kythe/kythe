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
    visibility = [
        "//visibility:public",
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
        "@boringssl//:crypto",
        "@com_github_gflags_gflags//:gflags",
        "@com_google_absl//absl/memory",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/types:optional",
        "@com_google_protobuf//:protobuf",
    ],
)

cc_library(
    name = "testlib",
    hdrs = [
        "RecordingOutputStream.h",
    ],
    deps = [":output"],
)
