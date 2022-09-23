package(
    default_visibility = ["//kythe:default_visibility"],
    features = ["layering_check"],
)

licenses(["notice"])  # The Universal Permissive License (UPL), Version 1.0

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
        "@com_google_absl//absl/log",
        "@souffle",
    ],
)
