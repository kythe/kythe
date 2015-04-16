licenses(["notice"])

package(default_visibility = ["//visibility:public"])

cc_library(
    name = "llvm",
    srcs = [
        "llvm/build/Release+Asserts/lib/libclang.a",
        "llvm/build/Release+Asserts/lib/libclangTooling.a",
        "llvm/build/Release+Asserts/lib/libclangFrontend.a",
        "llvm/build/Release+Asserts/lib/libclangDriver.a",
        "llvm/build/Release+Asserts/lib/libclangFrontendTool.a",
        "llvm/build/Release+Asserts/lib/libclangParse.a",
        "llvm/build/Release+Asserts/lib/libclangSema.a",
        "llvm/build/Release+Asserts/lib/libclangEdit.a",
        "llvm/build/Release+Asserts/lib/libclangRewrite.a",
        "llvm/build/Release+Asserts/lib/libclangFormat.a",
        "llvm/build/Release+Asserts/lib/libclangIndex.a",
        "llvm/build/Release+Asserts/lib/libclangSerialization.a",
        "llvm/build/Release+Asserts/lib/libclangAST.a",
        "llvm/build/Release+Asserts/lib/libclangLex.a",
        "llvm/build/Release+Asserts/lib/libclangBasic.a",
        "llvm/build/Release+Asserts/lib/libclangAnalysis.a",
        "llvm/build/Release+Asserts/lib/libLLVMOption.a",
        "llvm/build/Release+Asserts/lib/libLLVMTransformUtils.a",
        "llvm/build/Release+Asserts/lib/libLLVMMC.a",
        "llvm/build/Release+Asserts/lib/libLLVMMCParser.a",
        "llvm/build/Release+Asserts/lib/libLLVMBitReader.a",
        "llvm/build/Release+Asserts/lib/libLLVMSupport.a",
        "llvm/build/Release+Asserts/lib/libLLVMCore.a",
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
