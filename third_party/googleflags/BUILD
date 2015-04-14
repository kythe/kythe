package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # BSD

cc_library(
    name = "gflags",
    srcs = [
        "include/gflags/config.h",
        "src/gflags.cc",
        "src/gflags_completions.cc",
        "src/gflags_reporting.cc",
        "src/mutex.h",
        "src/util.h",
    ],
    hdrs = [
        "include/gflags/gflags.h",
        "include/gflags/gflags_completions.h",
        "include/gflags/gflags_declare.h",
    ],
    copts = [
        "-I./third_party/googleflags/include/gflags",
    ],
    includes = [
        "include",
    ],
)
