load("@//tools/build_rules/autotools:autoconf.bzl", "configure_generate")

package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # BSD

filegroup(
    name = "license",
    srcs = ["COPYING"],
)

cc_library(
    name = "glog",
    srcs = [
        "src/base/commandlineflags.h",
        "src/base/googleinit.h",
        "src/base/mutex.h",
        "src/demangle.cc",
        "src/demangle.h",
        "src/googletest.h",
        "src/logging.cc",
        "src/raw_logging.cc",
        "src/signalhandler.cc",
        "src/stacktrace.h",
        "src/stacktrace_generic-inl.h",
        "src/stacktrace_libunwind-inl.h",
        "src/stacktrace_powerpc-inl.h",
        "src/stacktrace_x86-inl.h",
        "src/stacktrace_x86_64-inl.h",
        "src/symbolize.cc",
        "src/symbolize.h",
        "src/utilities.cc",
        "src/utilities.h",
        "src/vlog_is_on.cc",
    ],
    hdrs = [
        "src/config.h",
        "src/glog/log_severity.h",
        "src/glog/logging.h",
        "src/glog/raw_logging.h",
        "src/glog/stl_logging.h",
        "src/glog/vlog_is_on.h",
    ],
    includes = [
        "src",
    ],
    deps = ["//external:gflags"],
)

configure_generate(
    name = "autoconf",
    srcs = glob(["**/*.in"]) + [
        "README",
    ],
    outs = [
        "src/config.h",
        "src/glog/logging.h",
        "src/glog/raw_logging.h",
        "src/glog/stl_logging.h",
        "src/glog/vlog_is_on.h",
    ],
    args = ["--quiet", "--disable-rtti"],
    deps = ["//external:gflags"],
)
