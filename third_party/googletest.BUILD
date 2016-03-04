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
        "include/gtest/gtest-message.h",
        "include/gtest/gtest-param-test.h",
        "include/gtest/gtest-printers.h",
        "include/gtest/gtest-spi.h",
        "include/gtest/gtest-test-part.h",
        "include/gtest/gtest-typed-test.h",
        "include/gtest/gtest_pred_impl.h",
        "include/gtest/gtest_prod.h",
        "include/gtest/internal/gtest-death-test-internal.h",
        "include/gtest/internal/gtest-filepath.h",
        "include/gtest/internal/gtest-internal.h",
        "include/gtest/internal/gtest-linked_ptr.h",
        "include/gtest/internal/gtest-param-util.h",
        "include/gtest/internal/gtest-param-util-generated.h",
        "include/gtest/internal/gtest-port.h",
        "include/gtest/internal/gtest-string.h",
        "include/gtest/internal/gtest-type-util.h",
        "src/gtest-internal-inl.h",
    ],
    copts = [
        "-Wno-non-virtual-dtor",
        "-Wno-unused-variable",
        "-Wno-implicit-fallthrough",
        "-Iexternal/gtest/include",
    ],
    includes = [
        "include",
    ],
    linkopts = [
        "-pthread",
    ],
)
