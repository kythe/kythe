licenses(["notice"])

load("/tools/build_rules/cc_resources", "cc_resources")

package(default_visibility = ["//visibility:public"])

filegroup(
    name = "license",
    srcs = ["LICENSE"],
)

cc_library(
    alwayslink=1,
    linkstatic=1,
    name = "llvm",
    srcs = [
        "llvm/build/Release+Asserts/lib/libclang.a",
        "llvm/build/Release+Asserts/lib/libclangTooling.a",
        "llvm/build/Release+Asserts/lib/libclangStaticAnalyzerFrontend.a",
        "llvm/build/Release+Asserts/lib/libclangStaticAnalyzerCore.a",
        "llvm/build/Release+Asserts/lib/libclangRewriteFrontend.a",
        "llvm/build/Release+Asserts/lib/libclangFrontend.a",
        "llvm/build/Release+Asserts/lib/libclangDriver.a",
        "llvm/build/Release+Asserts/lib/libclangFrontendTool.a",
        "llvm/build/Release+Asserts/lib/libclangARCMigrate.a",
        "llvm/build/Release+Asserts/lib/libclangParse.a",
        "llvm/build/Release+Asserts/lib/libclangSema.a",
        "llvm/build/Release+Asserts/lib/libclangEdit.a",
        "llvm/build/Release+Asserts/lib/libclangRewrite.a",
        "llvm/build/Release+Asserts/lib/libclangFormat.a",
        "llvm/build/Release+Asserts/lib/libclangIndex.a",
        "llvm/build/Release+Asserts/lib/libclangSerialization.a",
        "llvm/build/Release+Asserts/lib/libclangASTMatchers.a",
        "llvm/build/Release+Asserts/lib/libclangAST.a",
        "llvm/build/Release+Asserts/lib/libclangLex.a",
        "llvm/build/Release+Asserts/lib/libclangBasic.a",
        "llvm/build/Release+Asserts/lib/libclangAnalysis.a",
        "llvm/build/Release+Asserts/lib/libLLVMAnalysis.a",
        "llvm/build/Release+Asserts/lib/libLLVMOption.a",
        "llvm/build/Release+Asserts/lib/libLLVMTransformUtils.a",
        "llvm/build/Release+Asserts/lib/libLLVMMC.a",
        "llvm/build/Release+Asserts/lib/libLLVMMCParser.a",
        "llvm/build/Release+Asserts/lib/libLLVMBitReader.a",
        "llvm/build/Release+Asserts/lib/libLLVMSupport.a",
        "llvm/build/Release+Asserts/lib/libLLVMCore.a",
        # See https://github.com/google/bazel/issues/61
        "empty.cc",
    ],
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
    deps = [
        "//third_party/zlib",
    ],
)

cc_resources(
    name = "clang_builtin_headers_resources",
    data = glob(["llvm/build/Release+Asserts/lib/clang/3.7.0/include/**/*.h"]),
)
