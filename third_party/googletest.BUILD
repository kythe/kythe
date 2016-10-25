package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # BSD 3-clause

filegroup(
    name = "license",
    srcs = ["LICENSE"],
)

cc_library(
    name = "googletest",
    srcs = glob(
        [
            "src/*.cc",
            "src/*.h",
            "include/gtest/internal/**/*.h",
            "include/gtest/internal/*.h",
        ],
        exclude = [
            "src/gtest-all.cc",
        ],
    ),
    hdrs = glob(["include/gtest/*.h"]),
    copts = [
        "-Wno-non-virtual-dtor",
        "-Wno-unused-variable",
        "-Wno-implicit-fallthrough",
        "-Iexternal/com_github_google_googletest/include",
    ],
    includes = [
        "include",
    ],
    linkopts = [
        "-pthread",
    ],
)
