licenses(["notice"])

load("//tools:build_rules/cc_resources.bzl", "cc_resources")

package(default_visibility = ["//visibility:public"])

filegroup(
    name = "license",
    srcs = ["LICENSE"],
)

filegroup(
    name = "git",
    srcs = glob([
        "llvm/tools/clang/tools/extra/.git/**",
        "llvm/tools/clang/.git/**",
        "llvm/.git/**",
    ]),
)

cc_library(
    name = "llvm",
    # See https://github.com/google/bazel/issues/61 regarding check_modules's
    # modules_checked_empty.cc.
    # Do not sort; srcs are link-order dependent.
    srcs = [
        "llvm/build/lib/libclangFormat.a",
        "llvm/build/lib/libclangToolingInclusions.a",
        "llvm/build/lib/libclangToolingCore.a",
        "llvm/build/lib/libclangTooling.a",
        "llvm/build/lib/libclangFrontend.a",
        "llvm/build/lib/libclangDriver.a",
        "llvm/build/lib/libclangParse.a",
        "llvm/build/lib/libclangSema.a",
        "llvm/build/lib/libclangEdit.a",
        "llvm/build/lib/libclangRewrite.a",
        "llvm/build/lib/libclangSerialization.a",
        "llvm/build/lib/libclangAST.a",
        "llvm/build/lib/libclangLex.a",
        "llvm/build/lib/libclangBasic.a",
        "llvm/build/lib/libclangAnalysis.a",
        "llvm/build/lib/libLLVMMipsInfo.a",
        "llvm/build/lib/libLLVMAArch64Info.a",
        "llvm/build/lib/libLLVMARMInfo.a",
        "llvm/build/lib/libLLVMPowerPCInfo.a",
        "llvm/build/lib/libLLVMX86Info.a",
        "llvm/build/lib/libLLVMOption.a",
        "llvm/build/lib/libLLVMMC.a",
        "llvm/build/lib/libLLVMMCParser.a",
        "llvm/build/lib/libLLVMBitReader.a",
        "llvm/build/lib/libLLVMProfileData.a",
        "llvm/build/lib/libLLVMCore.a",
        "llvm/build/lib/libLLVMBinaryFormat.a",
        "llvm/build/lib/libLLVMSupport.a",
        "llvm/build/lib/libLLVMDemangle.a",
        "//tools/modules:check_modules",
    ],
    hdrs = glob([
        "llvm/include/**",
        "llvm/build/include/**",
        "llvm/build/tools/clang/include/**",
        "llvm/tools/clang/include/**",
    ]),
    defines = [
        "__STDC_LIMIT_MACROS",
        "__STDC_CONSTANT_MACROS",
    ],
    includes = [
        "llvm/build/include",
        "llvm/build/tools/clang/include",
        "llvm/include",
        "llvm/tools/clang/include",
    ],
    linkopts = [
        "-ldl",
        "-lcurses",
        "-lm",
        "-lpthread",
    ],
    linkstatic = 1,
    deps = [
        "//external:zlib",
    ],
    alwayslink = 1,
)

cc_resources(
    name = "clang_builtin_headers_resources",
    data = glob(["llvm/build/lib/clang/8.0.0/include/**/*.h"]),
)
