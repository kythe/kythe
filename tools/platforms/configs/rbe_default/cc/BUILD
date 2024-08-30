# Copyright 2016 The Bazel Authors. All rights reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# This becomes the BUILD file for @local_config_cc// under non-BSD unixes.

load(":cc_toolchain_config.bzl", "cc_toolchain_config")
load("@rules_cc//cc:defs.bzl", "cc_toolchain", "cc_toolchain_suite")

package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # Apache 2.0

cc_library(
    name = "malloc",
)

filegroup(
    name = "empty",
    srcs = [],
)

filegroup(
    name = "cc_wrapper",
    srcs = ["cc_wrapper.sh"],
)

filegroup(
    name = "compiler_deps",
    srcs = glob(["extra_tools/**"], allow_empty = True) + [":builtin_include_directory_paths"],
)

cc_toolchain(
    name = "cc-compiler-k8",
    toolchain_identifier = "linux_gnu_x86",
    toolchain_config = ":linux_gnu_x86",
    all_files = ":compiler_deps",
    ar_files = ":compiler_deps",
    as_files = ":compiler_deps",
    compiler_files = ":compiler_deps",
    dwp_files = ":empty",
    linker_files = ":compiler_deps",
    objcopy_files = ":empty",
    strip_files = ":empty",
    supports_param_files = 1,
    module_map = ":module.modulemap",
)

cc_toolchain_config(
    name = "linux_gnu_x86",
    cpu = "k8",
    compiler = "compiler",
    toolchain_identifier = "linux_gnu_x86",
    host_system_name = "i686-unknown-linux-gnu",
    target_system_name = "x86_64-unknown-linux-gnu",
    target_libc = "glibc_2.39",
    abi_version = "clang",
    abi_libc_version = "glibc_2.39",
    cxx_builtin_include_directories = [
        "/usr/include",
        "/usr/include/c++/13",
        "/usr/include/c++/13/backward",
        "/usr/include/x86_64-linux-gnu",
        "/usr/include/x86_64-linux-gnu/c++/13",
        "/usr/lib/llvm-15/lib/clang/15.0.7/include",
        "/usr/local/include",
    ],
    tool_paths = {
        "ar": "/usr/bin/ar",
        "ld": "/usr/bin/ld",
        "llvm-cov": "None",
        "llvm-profdata": "None",
        "cpp": "/usr/bin/cpp",
        "gcc": "/usr/lib/llvm-15/bin/clang",
        "dwp": "/usr/bin/dwp",
        "gcov": "/usr/bin/gcov",
        "nm": "/usr/bin/nm",
        "objcopy": "/usr/bin/objcopy",
        "objdump": "/usr/bin/objdump",
        "strip": "/usr/bin/strip"
    },
    compile_flags = [
        "-fstack-protector",
        "-Wall",
        "-Wthread-safety",
        "-Wself-assign",
        "-Wunused-but-set-parameter",
        "-Wno-free-nonheap-object",
        "-fcolor-diagnostics",
        "-fno-omit-frame-pointer"
    ],
    opt_compile_flags = [
        "-g0",
        "-O2",
        "-D_FORTIFY_SOURCE=1",
        "-DNDEBUG",
        "-ffunction-sections",
        "-fdata-sections"
    ],
    dbg_compile_flags = ["-g"],
    cxx_flags = ["-std=c++14"],
    link_flags = [
        "-fuse-ld=/usr/bin/ld.gold",
        "-Wl,-no-as-needed",
        "-Wl,-z,relro,-z,now",
        "-B/usr/lib/llvm-15/bin",
    ],
    link_libs = ["-lstdc++", "-lm"],
    opt_link_flags = ["-Wl,--gc-sections"],
    unfiltered_compile_flags = [
        "-no-canonical-prefixes",
        "-Wno-builtin-macro-redefined",
        "-D__DATE__=\"redacted\"",
        "-D__TIMESTAMP__=\"redacted\"",
        "-D__TIME__=\"redacted\""
    ],
    coverage_compile_flags = ["--coverage"],
    coverage_link_flags = ["--coverage"],
    supports_start_end_lib = True,
)
