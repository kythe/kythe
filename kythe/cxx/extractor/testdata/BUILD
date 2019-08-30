load(
    "//tools/build_rules/verifier_test:cc_indexer_test.bzl",
    "cc_bazel_extractor_test",
    "cc_extractor_test",
    "objc_bazel_extractor_test",
)
load("//tools:build_rules/testing.bzl", "shell_tool_test")
load("//tools:build_rules/shims.bzl", "go_test")

package(default_visibility = ["//kythe:default_visibility"])

filegroup(
    name = "skip",
    srcs = ["skip_functions.sh"],
)

go_test(
    name = "root_directory_test",
    srcs = ["root_directory_test.go"],
    data = [
        "altroot/altpath/file.cc",
        "//kythe/cxx/extractor:cxx_extractor",
    ],
    deps = [
        "//kythe/go/platform/kzip",
        "//kythe/proto:analysis_go_proto",
        "//kythe/proto:filecontext_go_proto",
        "//kythe/proto:storage_go_proto",
        "@com_github_golang_protobuf//proto:go_default_library",
        "@com_github_golang_protobuf//ptypes:go_default_library",
        "@com_github_golang_protobuf//ptypes/any:go_default_library",
        "@com_github_google_go_cmp//cmp:go_default_library",
        "@io_bazel_rules_go//go/tools/bazel:go_default_library",
    ],
)

shell_tool_test(
    name = "root_directory",
    data = [
        "altroot/altpath/file.cc",
        "altroot_altpath_file.UNIT",
        "skip_functions.sh",
        "test_common.sh",
    ],
    scriptfile = "test_root_directory.sh",
    tools = {
        "EXTRACTOR": "//kythe/cxx/extractor:cxx_extractor",
        "KINDEX_TOOL": "//kythe/cxx/tools:kindex_tool",
    },
)

shell_tool_test(
    name = "alternate_platform",
    data = [
        "arm.UNIT",
        "arm.cc",
        "skip_functions.sh",
        "test_common.sh",
    ],
    scriptfile = "test_alternate_platform.sh",
    tools = {
        "EXTRACTOR": "//kythe/cxx/extractor:cxx_extractor",
        "KINDEX_TOOL": "//kythe/cxx/tools:kindex_tool",
    },
)

shell_tool_test(
    name = "main_source_file_no_env_dep",
    data = [
        "main_source_file_no_env_dep.cc",
        "main_source_file_no_env_dep_with.UNIT",
        "main_source_file_no_env_dep_without.UNIT",
        "skip_functions.sh",
        "test_common.sh",
    ],
    scriptfile = "test_main_source_file_no_env_dep.sh",
    tools = {
        "EXTRACTOR": "//kythe/cxx/extractor:cxx_extractor",
        "KINDEX_TOOL": "//kythe/cxx/tools:kindex_tool",
    },
)

shell_tool_test(
    name = "main_source_file_env_dep",
    data = [
        "main_source_file_env_dep.cc",
        "main_source_file_env_dep_with.UNIT",
        "main_source_file_env_dep_without.UNIT",
        "skip_functions.sh",
        "test_common.sh",
    ],
    scriptfile = "test_main_source_file_env_dep.sh",
    tools = {
        "EXTRACTOR": "//kythe/cxx/extractor:cxx_extractor",
        "KINDEX_TOOL": "//kythe/cxx/tools:kindex_tool",
    },
)

shell_tool_test(
    name = "extract_transcript",
    data = [
        "skip_functions.sh",
        "test_common.sh",
        "transcript_a.h",
        "transcript_b.h",
        "transcript_main.UNIT",
        "transcript_main.cc",
    ],
    scriptfile = "test_extract_transcript.sh",
    tools = {
        "EXTRACTOR": "//kythe/cxx/extractor:cxx_extractor",
        "KINDEX_TOOL": "//kythe/cxx/tools:kindex_tool",
    },
)

cc_test(
    name = "claim_pragma_test",
    size = "small",
    srcs = ["claim_pragma_test.cc"],
    data = [
        "claim_a.h",
        "claim_b.h",
        "claim_main.cc",
    ],
    deps = [
        "//kythe/cxx/extractor:lib",
        "//kythe/cxx/extractor:supported_language",
        "//third_party:gmock_main",
        "//third_party:gtest",
        "@com_google_absl//absl/memory",
        "@com_google_protobuf//:protobuf",
    ],
)

shell_tool_test(
    name = "stdin_names",
    data = [
        "stdin.vnames",
        "test_common.sh",
        "test_stdin_names_verify.cc",
    ],
    scriptfile = "test_stdin_names.sh",
    tools = {
        "EXTRACTOR": "//kythe/cxx/extractor:cxx_extractor",
        "INDEXER": "//kythe/cxx/indexer/cxx:indexer",
        "VERIFIER": "//kythe/cxx/verifier",
    },
)

shell_tool_test(
    name = "kzip",
    data = [
        "skip_functions.sh",
        "test_common.sh",
        "transcript_a.h",
        "transcript_b.h",
        "transcript_main.cc",
    ],
    scriptfile = "test_kzip.sh",
    tools = {
        "EXTRACTOR": "//kythe/cxx/extractor:cxx_extractor",
    },
)

shell_tool_test(
    name = "has_include",
    data = [
        "has_include_test.UNIT",
        "has_include_test.cc",
        "has_include_test.h",
        "skip_functions.sh",
        "test_common.sh",
    ],
    scriptfile = "test_has_include.sh",
    tools = {
        "EXTRACTOR": "//kythe/cxx/extractor:cxx_extractor",
        "KINDEX_TOOL": "//kythe/cxx/tools:kindex_tool",
    },
)

shell_tool_test(
    name = "indirect_has_include",
    data = [
        "has_include_test.h",
        "indirect_has_include_test.UNIT",
        "indirect_has_include_test.cc",
        "indirect_has_include_test.h",
        "skip_functions.sh",
        "test_common.sh",
    ],
    scriptfile = "test_indirect_has_include.sh",
    tools = {
        "EXTRACTOR": "//kythe/cxx/extractor:cxx_extractor",
        "KINDEX_TOOL": "//kythe/cxx/tools:kindex_tool",
    },
)

shell_tool_test(
    name = "metadata",
    data = [
        "metadata.UNIT",
        "metadata.cc",
        "metadata.cc.meta",
        "skip_functions.sh",
        "test_common.sh",
    ],
    scriptfile = "test_metadata.sh",
    tools = {
        "EXTRACTOR": "//kythe/cxx/extractor:cxx_extractor",
        "KINDEX_TOOL": "//kythe/cxx/tools:kindex_tool",
    },
)

shell_tool_test(
    name = "modules",
    data = [
        "modfoo.h",
        "modfoo.modulemap",
        "modules.UNIT",
        "modules.cc",
        "skip_functions.sh",
        "test_common.sh",
    ],
    scriptfile = "test_modules.sh",
    tools = {
        "EXTRACTOR": "//kythe/cxx/extractor:cxx_extractor",
        "KINDEX_TOOL": "//kythe/cxx/tools:kindex_tool",
    },
)

shell_tool_test(
    name = "build_config",
    data = [
        "build_config.UNIT",
        "build_config.cc",
        "skip_functions.sh",
        "test_common.sh",
    ],
    scriptfile = "test_build_config.sh",
    tools = {
        "EXTRACTOR": "//kythe/cxx/extractor:cxx_extractor",
        "KINDEX_TOOL": "//kythe/cxx/tools:kindex_tool",
    },
)

shell_tool_test(
    name = "installed_dir",
    data = [
        # Clang resolves the include directory relative to `bin`,
        # so the former needs to exist.
        "bin/clang",
        "include/c++/v1/dummy",
        "installed_dir.UNIT",
        "installed_dir.cc",
        "skip_functions.sh",
        "test_common.sh",
    ],
    scriptfile = "test_installed_dir.sh",
    tools = {
        "EXTRACTOR": "//kythe/cxx/extractor:cxx_extractor",
        "KINDEX_TOOL": "//kythe/cxx/tools:kindex_tool",
    },
)

cc_extractor_test(
    name = "recursish_macro_test",
    size = "small",
    srcs = [
        "recursish_macro.cc",
    ],
)

cc_extractor_test(
    name = "extract_verify_test",
    size = "small",
    srcs = [
        "extract_verify_test.cc",
    ],
)

cc_extractor_test(
    name = "extract_verify_stddef_test",
    size = "small",
    srcs = [
        "extract_verify_stddef_test.cc",
    ],
)

cc_extractor_test(
    name = "extract_verify_std_string_test",
    size = "small",
    srcs = [
        "extract_verify_std_string_test.cc",
    ],
)

cc_extractor_test(
    name = "extract_verify_std_vector_test",
    size = "small",
    srcs = [
        "extract_verify_std_vector_test.cc",
    ],
)

cc_extractor_test(
    name = "extract_verify_iostream_test",
    size = "small",
    srcs = [
        "extract_verify_iostream_test.cc",
    ],
)

objc_library(
    name = "objc_lib",
    srcs = ["objc_lib.m"],
    # This is a little tricky to build, so don't do it automatically.
    tags = [
        "arc-ignore",
        "broken",
        "manual",
    ],
)

# Test that we can use the bazel extractor to extract a .xa file, index the
# result of the extraction, and verify the result of the indexing. The primary
# purpose of this test is to see if the whole pipeline works without failure.
# The actual verficiation being done is not important as long as *some*
# verficiation is being done.
#
# The xa file is generated and copied into this directory by running the helper
# script run_objc_bazel_extraction.sh.
objc_bazel_extractor_test(
    name = "objc_lib_test",
    size = "small",
    src = "objc_lib.m",
    data = "objc_lib.xa",
    restricted_to = ["//buildenv:darwin"],
    # This test no longer passes, even on darwin, so do not include it in our presubmit.
    tags = [
        "arc-ignore",
        "broken",
        "manual",
    ],
)

cc_library(
    name = "cc_lib",
    srcs = ["cc_lib.cc"],
)

# Test that we can use the bazel extractor to extract a .xa file, index the
# result of the extraction, and verify the result of the indexing. The primary
# purpose of this test is to see if the whole pipeline works without failure.
# The actual verficiation being done is not important.
#
# The xa file is generated and copied into this directory by running the helper
# script run_cxx_bazel_extraction.sh.
cc_bazel_extractor_test(
    name = "cc_lib_test",
    size = "small",
    src = "cc_lib.cc",
    data = "cc_lib.xa",
)
