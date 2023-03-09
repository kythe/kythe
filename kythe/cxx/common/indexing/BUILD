load("//:visibility.bzl", "PUBLIC_VISIBILITY")

package(
    default_visibility = ["//kythe:default_visibility"],
    features = ["layering_check"],
)

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
        "//kythe/cxx/common:sha256_hasher",
        "//kythe/proto:analysis_cc_proto",
        "//kythe/proto:common_cc_proto",
        "//kythe/proto:storage_cc_proto",
        "@boringssl//:crypto",
        "@com_github_google_glog//:glog",
        "@com_google_absl//absl/memory",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/strings:str_format",
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
