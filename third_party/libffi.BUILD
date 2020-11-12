# Copyright 2019 Google LLC
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

cc_library(
    name = "libffi",
    srcs = [
        "src/closures.c",
        "src/debug.c",
        "src/java_raw_api.c",
        "src/prep_cif.c",
        "src/raw_api.c",
        "src/types.c",
    ] + select({
        "@bazel_tools//src/conditions:linux_x86_64": [
            "src/x86/asmnames.h",
            "src/x86/ffi.c",
            "src/x86/ffi64.c",
            "src/x86/ffiw64.c",
            "src/x86/internal.h",
            "src/x86/internal64.h",
            "src/x86/sysv.S",
            "src/x86/unix64.S",
            "src/x86/win64.S",
        ],
        "@bazel_tools//src/conditions:linux_ppc": [
            "src/powerpc/ffi.c",
            "src/powerpc/ffi_linux64.c",
            "src/powerpc/ffi_sysv.c",
            "src/powerpc/linux64.S",
            "src/powerpc/linux64_closure.S",
            "src/powerpc/ppc_closure.S",
            "src/powerpc/sysv.S",
        ],
        "@bazel_tools//src/conditions:linux_aarch64": [
            "src/aarch64/ffi.c",
            "src/aarch64/internal.h",
            "src/aarch64/sysv.S",
        ],
        "//conditions:default": [
        ],
    }),
    hdrs = [
        "configure-bazel-gen/fficonfig.h",
        "configure-bazel-gen/include/ffi.h",
        "configure-bazel-gen/include/ffitarget.h",
        "include/ffi_cfi.h",
        "include/ffi_common.h",
    ],
    copts = [
        # libffi-3.3-rc0 uses variable length arrays for closures on all
        # platforms.
        "-Wno-vla",
        # libffi does not check the result of ftruncate.
        "-Wno-unused-result",
    ],
    includes = [
        "configure-bazel-gen",
        "configure-bazel-gen/include",
        "include",
    ],
    textual_hdrs = ["src/dlmalloc.c"],
    visibility = ["//visibility:public"],
)
