package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # BSD 3-clause

filegroup(
    name = "license",
    srcs = ["LICENSE"],
)

cc_library(
    name = "googletest",
    srcs = [
        "src/gtest.cc",
        "src/gtest-death-test.cc",
        "src/gtest-filepath.cc",
        "src/gtest-port.cc",
        "src/gtest-printers.cc",
        "src/gtest-test-part.cc",
        "src/gtest-typed-test.cc",
    ],
    hdrs = [
        "include/gtest/gtest.h",
        "include/gtest/gtest-death-test.h",
        "include/gtest/gtest-typed-test.h",
    ],
    copts = [
        "-Wno-non-virtual-dtor",
        "-Wno-unused-variable",
        "-Wno-implicit-fallthrough",
        "-Ithird_party/googletest",
    ],
    includes = [
        "include",
    ],
)
