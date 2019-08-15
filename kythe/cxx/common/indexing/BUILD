package(default_visibility = ["//kythe:default_visibility"])

load("//:visibility.bzl", "PUBLIC_VISIBILITY")

cc_library(
    name = "output",
    srcs = [
        "KytheGraphRecorder.cc",
    ],
    hdrs = [
        "KytheGraphRecorder.h",
        "KytheOutputStream.h",
    ],
    visibility = ["//visibility:public"],
    deps = [
        "//kythe/proto:analysis_cc_proto",
        "//kythe/proto:common_cc_proto",
        "//kythe/proto:storage_cc_proto",
        "@com_google_absl//absl/memory",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/strings:str_format",
        "@com_google_absl//absl/types:optional",
        "@com_google_protobuf//:protobuf",
    ],
)

cc_library(
    name = "caching_output",
    srcs = [
        "KytheCachingOutput.cc",
        "MemcachedHashCache.cc",
    ],
    hdrs = [
        "KytheCachingOutput.h",
        "MemcachedHashCache.h",
    ],
    visibility = [PUBLIC_VISIBILITY],
    deps = [
        ":output",
        "//external:libmemcached",
        "//kythe/proto:analysis_cc_proto",
        "//kythe/proto:common_cc_proto",
        "//kythe/proto:storage_cc_proto",
        "@boringssl//:crypto",
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
