package(
    default_visibility = ["//visibility:public"],
)

TARGET_DEFAULTS = {
    "LLVMSupport": {
        "linkopts": [
            "-pthread",
            "-ldl",
        ],
        "deps": [
            "@//external:zlib",
        ],
    },
    "LLVMDebugInfoCodeView": {
        "deps": [
            ":LLVMBinaryFormat",
        ],
    },
    "LLVMCore": {
        "additional_header_dirs": [
            # Layering violation.
            "/root/include/llvm/Analysis",
        ],
        "hdrs": glob([
            "include/llvm/*.h",
        ]),
    },
    "LLVMRemarks": {
        # Technically BitstreamWriter, but it's header-only
        # and BitstreamReader is equivalent.
        "deps": [":LLVMBitstreamReader"],
    },
    "LLVMScalarOpts": {
        "deps": [":LLVMTarget"],
    },
    "LLVMTransformUtils": {
        "hdrs": glob(["include/llvm-c/Transforms/**/*.h"]),
    },
    "LLVMX86CodeGen": {
        "deps": [":LLVMipo"],
    },
    "clangAST": {
        "textual_hdrs": [
            ":tools_clang_include_clang_AST_genhdrs",
        ],
    },
    "clangBasic": {
        "deps": [
            ":LLVMTarget",
        ],
        "textual_hdrs": [
            "tools/clang/include/clang/Basic/Version.inc",
            ":tools_clang_include_clang_Basic_genhdrs",
        ],
    },
    "clangCodeGen": {
        "deps": [":all_targets"],
    },
    "clangDriver": {
        "textual_hdrs": [
            ":tools_clang_include_clang_StaticAnalyzer_Checkers_genhdrs",
        ],
    },
    "clangFrontend": {
        "deps": [
            ":LLVMLinker",
        ],
        "hdrs": [
            "tools/clang/include/clang/StaticAnalyzer/Core/AnalyzerOptions.h",
        ],
        "textual_hdrs": [
            "tools/clang/include/clang/StaticAnalyzer/Core/Analyses.def",
            "tools/clang/include/clang/StaticAnalyzer/Core/AnalyzerOptions.def",
        ],
    },
    "clangIndex": {
        "textual_hdrs": [
            "tools/clang/include/clang/StaticAnalyzer/Core/Analyses.def",
        ],
    },
    "clangParse": {
        "textual_hdrs": [
            ":tools_clang_include_clang_Parse_genhdrs",
        ],
    },
    "clangSema": {
        "textual_hdrs": [
            ":tools_clang_include_clang_Sema_genhdrs",
        ],
        "deps": [":all_targets"],
    },
    "clangSerialization": {
        "textual_hdrs": [
            ":tools_clang_include_clang_Serialization_genhdrs",
        ],
    },
}

cc_library(
    name = "clang-c",
    hdrs = glob(["tools/clang/include/clang-c/*.h"]) + [
        "tools/clang/include/clang/Config/config.h",
    ],
    includes = [
        "tools/clang/include",
    ],
)

cc_library(
    name = "llvm-c",
    hdrs = glob([
        "include/llvm-c/*.h",
    ]) + [
        "include/llvm/Config/abi-breaking.h",
        "include/llvm/Config/config.h",
        "include/llvm/Config/llvm-config.h",
        "include/llvm/Config/AsmParsers.def",
        "include/llvm/Config/AsmPrinters.def",
        "include/llvm/Config/Disassemblers.def",
        "include/llvm/Config/Targets.def",
    ],
    includes = [
        "include",
    ],
)

genrule(
    name = "llvm_vcsrevision_h_gen",
    outs = ["include/llvm/Support/VCSRevision.h"],
    cmd = "touch $@",
)

cc_library(
    name = "llvm_vcsrevision_h",
    hdrs = ["include/llvm/Support/VCSRevision.h"],
)

genrule(
    name = "clang_basic_version_inc_gen",
    outs = ["tools/clang/lib/Basic/VCSVersion.inc"],
    cmd = ("printf " +
           "\"#define CLANG_VERSION 9999.0\n\"" +
           "\"#define CLANG_VERSION_MAJOR 9999\n\"" +
           "\"#define CLANG_VERSION_MINOR 0\n\"" +
           "\"#define CLANG_VERSION_PATCHLEVEL 0\n\"" +
           "\"#define CLANG_VERSION_STRING \\\"google3-trunk\\\"\n\" > $@"),
)

load("@io_kythe//tools:build_rules/cc_resources.bzl", "cc_resources")

builtin_headers = glob(
    ["tools/clang/lib/Headers/**"],
    exclude = ["tools/clang/lib/Headers/**/CMakeLists.txt"],
)

genrule(
    name = "builtin_headers_gen",
    srcs = builtin_headers,
    outs = [hdr.replace("lib/Headers/", "staging/include/") for hdr in builtin_headers],
    cmd = """
      SRCS=($(SRCS))
      OUTS=($(OUTS))
      for i in "$${!SRCS[@]}"; do
        cp $${SRCS[$$i]} $${OUTS[$$i]}
      done""",
    output_to_bindir = True,
)

cc_resources(
    name = "clang_builtin_headers_resources",
    data = [":builtin_headers_gen"],
    strip = "staging/include/",
)

load("@io_kythe//tools/build_rules/llvm:cmake_defines.bzl", "cmake_defines", "LLVM_TARGETS")

cc_library(
    name = "all_targets",
    deps = [":LLVM%sCodeGen" % t for t in LLVM_TARGETS],
)

load("@io_kythe_llvmbzlgen//rules:llvmbuild.bzl", _llvmbuild_context = "make_context")
load("@io_kythe//tools/build_rules/llvm:generated_llvm_build_targets.bzl", "generated_llvm_build_targets")
load("@io_kythe//tools/build_rules/llvm:llvm.bzl", "make_context")
load("@io_kythe//tools/build_rules/llvm:generated_cmake_targets.bzl", "generated_cmake_targets")

generated_cmake_targets(make_context(
    cmake_defines = cmake_defines(),
    llvmbuildctx = generated_llvm_build_targets(_llvmbuild_context()),
    target_defaults = TARGET_DEFAULTS,
))
