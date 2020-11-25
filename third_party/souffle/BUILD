package(
    default_visibility = ["//kythe:default_visibility"],
    features = ["layering_check"],
)

licenses(["notice"])  # The Universal Permissive License (UPL), Version 1.0

load("//:visibility.bzl", "PUBLIC_VISIBILITY")

filegroup(
    name = "license",
    srcs = ["LICENSE"],
)

cc_library(
    name = "parse_transform",
    srcs = [
        "parse_transform.cc",
    ],
    hdrs = [
        "parse_transform.h",
    ],
    deps = [
        "@com_github_google_glog//:glog",
        "@souffle",
    ],
)
