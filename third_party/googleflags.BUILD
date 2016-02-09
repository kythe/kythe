package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # BSD

filegroup(
    name = "license",
    srcs = ["COPYING.txt"],
)

genrule(
    name = "headers",
    srcs =
        glob([
            "cmake/*",
            "src/*.in",
        ]) + ["CMakeLists.txt"] + [
            "src/gflags.cc",
            "src/gflags_reporting.cc",
            "src/gflags_completions.cc",
            "src/util.h",
            "src/mutex.h",
        ],
    outs = [
        "include/gflags/config.h",
        "include/gflags/gflags_completions.h",
        "include/gflags/gflags_declare.h",
        "include/gflags/gflags_gflags.h",
        "include/gflags/gflags.h",
    ],
    # `yes` will always fail, so only fail if cmake does.
    # TODO(shahms): Pull this out into a macro/rule.
    cmd = "ROOT=`pwd`; (cd $(@D); set +o pipefail; " +
          "yes \"\" | cmake -i $$ROOT/$$(dirname $(location :CMakeLists.txt)))",
    visibility = ["//visibility:private"],
)

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
        "include/gflags/gflags_gflags.h",
    ],
    copts = [
        "-Wno-unused-local-typedef",
        "-Wno-unknown-warning-option",
        "-I$(GENDIR)/external/googleflags/include/gflags",
    ],
    includes = [
        "include",
    ],
)
