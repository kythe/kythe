package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # BSD 3-clause

filegroup(
    name = "license",
    srcs = ["LICENSE"],
)

cc_library(
    name = "gtest",
    srcs = glob(
        [
            "googletest/src/*.cc",
            "googletest/src/*.h",
            "googletest/include/gtest/internal/**/*.h",
            "googletest/include/gtest/internal/*.h",
        ],
        exclude = [
            "googletest/src/gtest-all.cc",
            "googletest/src/gtest_main.cc",
        ],
    ),
    hdrs = glob(["googletest/include/gtest/*.h"]),
    copts = [
        "-Wno-non-virtual-dtor",
        "-Wno-unused-variable",
        "-Wno-implicit-fallthrough",
        "-Iexternal/com_github_google_googletest/googletest/",
    ],
    includes = [
        "googletest/include",
    ],
    linkopts = [
        "-pthread",
    ],
)

cc_library(
    name = "gtest_main",
    srcs = ["googletest/src/gtest_main.cc"],
    hdrs = glob(["googletest/include/gtest/*.h"]),
    deps = [
        ":gtest",
    ],
)

cc_library(
    name = "gmock",
    srcs = glob(
        [
            "googlemock/src/*.cc",
            "googlemock/src/*.h",
            "googlemock/include/gmock/internal/**/*.h",
            "googlemock/include/gmock/internal/*.h",
        ],
        exclude = [
            "googlemock/src/gmock_main.cc",
            "googlemock/src/gmock-all.cc",
        ],
    ),
    hdrs = glob(["googlemock/include/gmock/*.h"]),
    copts = [
        "-Wno-non-virtual-dtor",
        "-Wno-unused-variable",
        "-Wno-implicit-fallthrough",
        "-Iexternal/com_github_google_googletest/googlemock/",
    ],
    includes = [
        "googlemock/include",
    ],
    linkopts = [
        "-pthread",
    ],
    deps = [":gtest"],
)

cc_library(
    name = "gmock_main",
    srcs = ["googlemock/src/gmock_main.cc"],
    hdrs = glob(["googlemock/include/gmock/*.h"]),
    deps = [
        ":gmock",
    ],
)
