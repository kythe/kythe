package(default_visibility = ["//visibility:public"])

licenses(["unencumbered"])  # Authored by Google, Apache 2.0

# TODO(bazel-team): The MacOS tools give an error if there is no object file on
# the command line to libtool, so we use an empty .cc file here. We should find
# a better way to handle this case.
cc_library(
    name = "malloc",
    srcs = ["empty.cc"],
)

cc_library(
    name = "stl",
    srcs = ["empty.cc"],
)

filegroup(
    name = "empty",
    srcs = [],
)

# This is the entry point for --crosstool_top.
#
# The cc_toolchain rule used is found by:
#
# 1. Finding the appropriate toolchain in the CROSSTOOL file based on the --cpu
#    and --compiler command line flags (if they exist, otherwise using the
#    "default_target_cpu" / "default_toolchain" fields in the CROSSTOOL file)
# 2. Concatenating the "target_cpu" and "compiler" fields of the toolchain in
#    use and using that as a key in the map in the "toolchains" attribute
cc_toolchain_suite(
    name = "toolchain",
    toolchains = {
        "darwin|clang": ":cc-compiler-darwin",
        "local|clang": ":cc-compiler-local",
        "armeabi-v7a|false": ":cc-compiler-armeabi-v7a",
        "ios_x86_64|compiler": ":cc-compiler-ios_x86_64",
    },
)

cc_toolchain(
    name = "cc-compiler-local",
    all_files = ":empty",
    compiler_files = ":empty",
    cpu = "local",
    dwp_files = ":empty",
    dynamic_runtime_libs = [":empty"],
    linker_files = ":empty",
    objcopy_files = ":empty",
    static_runtime_libs = [":empty"],
    strip_files = ":empty",
    supports_param_files = 0,
)

cc_toolchain(
    name = "cc-compiler-armeabi-v7a",
    all_files = ":empty",
    compiler_files = ":empty",
    cpu = "local",
    dwp_files = ":empty",
    dynamic_runtime_libs = [":empty"],
    linker_files = ":empty",
    objcopy_files = ":empty",
    static_runtime_libs = [":empty"],
    strip_files = ":empty",
    supports_param_files = 0,
)

cc_toolchain(
    name = "cc-compiler-k8",
    all_files = ":empty",
    compiler_files = ":empty",
    cpu = "local",
    dwp_files = ":empty",
    dynamic_runtime_libs = [":empty"],
    linker_files = ":empty",
    objcopy_files = ":empty",
    static_runtime_libs = [":empty"],
    strip_files = ":empty",
    supports_param_files = 0,
)

cc_toolchain(
    name = "cc-compiler-darwin",
    all_files = ":darwin_files",
    compiler_files = ":darwin_files",
    cpu = "darwin",
    dwp_files = ":empty",
    dynamic_runtime_libs = [":empty"],
    linker_files = ":darwin_files",
    objcopy_files = ":empty",
    static_runtime_libs = [":empty"],
    strip_files = ":empty",
    supports_param_files = 0,
)

cc_toolchain(
    name = "cc-compiler-ios_x86_64",
    all_files = ":empty",
    compiler_files = ":empty",
    cpu = "local",
    dwp_files = ":empty",
    dynamic_runtime_libs = [":empty"],
    linker_files = ":empty",
    objcopy_files = ":empty",
    static_runtime_libs = [":empty"],
    strip_files = ":empty",
    supports_param_files = 0,
)

filegroup(
    name = "srcs",
    srcs = glob(["**"]),
)

filegroup(
    name = "darwin_files",
    srcs = [
        ":clang_wrapper_is_not_clang",
        ":clang_wrapper_is_not_clang++",
    ],
)
