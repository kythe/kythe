package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # Apache 2.0

cc_library(
    name = "zip",
    srcs = ["zip.cc"],
    hdrs = [
        "common.h",
        "zip.h",
    ],
    copts = ["-Wno-unused-private-field"],
    deps = [
        "//third_party/zlib",
    ],
)

cc_binary(
    name = "zipper",
    srcs = ["zip_main.cc"],
    deps = [":zip"],
)

cc_binary(
    name = "ijar",
    srcs = [
        "classfile.cc",
        "ijar.cc",
    ],
    copts = ["-Wno-unused-private-field"],
    deps = [":zip"],
)
