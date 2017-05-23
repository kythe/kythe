load(
    "//tools:build_rules/kythe.bzl",
    "cc_verifier_test",
    "objc_bazel_verifier_test",
    "cc_bazel_verifier_test",
)

package(default_visibility = ["//kythe:default_visibility"])

filegroup(
    name = "skip",
    srcs = ["skip_functions.sh"],
)

sh_test(
    name = "root_directory",
    size = "small",
    srcs = [
        "test_root_directory.sh",
    ],
    data = [
        "altroot/altpath/file.cc",
        "altroot_altpath_file.UNIT",
        "skip_functions.sh",
        "test_common.sh",
        "//kythe/cxx/extractor:cxx_extractor",
        "//kythe/cxx/tools:kindex_tool",
    ],
)

sh_test(
    name = "alternate_platform",
    size = "small",
    srcs = [
        "test_alternate_platform.sh",
    ],
    data = [
        "arm.UNIT",
        "arm.cc",
        "skip_functions.sh",
        "test_common.sh",
        "//kythe/cxx/extractor:cxx_extractor",
        "//kythe/cxx/tools:kindex_tool",
    ],
)

sh_test(
    name = "main_source_file_no_env_dep",
    size = "small",
    srcs = [
        "test_main_source_file_no_env_dep.sh",
    ],
    data = [
        "main_source_file_no_env_dep.cc",
        "main_source_file_no_env_dep_with.UNIT",
        "main_source_file_no_env_dep_without.UNIT",
        "skip_functions.sh",
        "test_common.sh",
        "//kythe/cxx/extractor:cxx_extractor",
        "//kythe/cxx/tools:kindex_tool",
    ],
)

sh_test(
    name = "main_source_file_env_dep",
    size = "small",
    srcs = [
        "test_main_source_file_env_dep.sh",
    ],
    data = [
        "main_source_file_env_dep.cc",
        "main_source_file_env_dep_with.UNIT",
        "main_source_file_env_dep_without.UNIT",
        "skip_functions.sh",
        "test_common.sh",
        "//kythe/cxx/extractor:cxx_extractor",
        "//kythe/cxx/tools:kindex_tool",
    ],
)

sh_test(
    name = "extract_transcript",
    size = "small",
    srcs = [
        "test_extract_transcript.sh",
    ],
    data = [
        "skip_functions.sh",
        "test_common.sh",
        "transcript_a.h",
        "transcript_b.h",
        "transcript_main.UNIT",
        "transcript_main.cc",
        "//kythe/cxx/extractor:cxx_extractor",
        "//kythe/cxx/tools:kindex_tool",
    ],
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
        "//kythe/cxx/common:supported_language",
        "//kythe/cxx/extractor:lib",
        "//third_party:gmock_main",
        "//third_party:gtest",
        "//third_party/proto:protobuf",
    ],
)

sh_test(
    name = "stdin_names",
    size = "small",
    srcs = [
        "test_stdin_names.sh",
    ],
    data = [
        "skip_functions.sh",
        "stdin.vnames",
        "test_common.sh",
        "test_stdin_names_verify.cc",
        "//kythe/cxx/extractor:cxx_extractor",
        "//kythe/cxx/indexer/cxx:indexer",
        "//kythe/cxx/verifier",
        "//kythe/go/platform/tools/indexpack",
    ],
)

sh_test(
    name = "index_pack",
    size = "small",
    srcs = [
        "test_index_pack.sh",
    ],
    data = [
        "skip_functions.sh",
        "test_common.sh",
        "transcript_a.h",
        "transcript_b.h",
        "transcript_main.cc",
        "//kythe/cxx/extractor:cxx_extractor",
        "//kythe/cxx/tools:kindex_tool",
    ],
)

sh_test(
    name = "metadata",
    size = "small",
    srcs = [
        "test_metadata.sh",
    ],
    data = [
        "metadata.UNIT",
        "metadata.cc",
        "metadata.cc.meta",
        "skip_functions.sh",
        "test_common.sh",
        "//kythe/cxx/extractor:cxx_extractor",
        "//kythe/cxx/tools:kindex_tool",
    ],
)

sh_test(
    name = "modules",
    size = "small",
    srcs = [
        "test_modules.sh",
    ],
    data = [
        "modfoo.h",
        "modfoo.modulemap",
        "modules.UNIT",
        "modules.cc",
        "skip_functions.sh",
        "test_common.sh",
        "//kythe/cxx/extractor:cxx_extractor",
        "//kythe/cxx/tools:kindex_tool",
    ],
)

cc_verifier_test(
    name = "recursish_macro_test",
    size = "small",
    srcs = [
        "recursish_macro.cc",
    ],
)

cc_verifier_test(
    name = "extract_verify_test",
    size = "small",
    srcs = [
        "extract_verify_test.cc",
    ],
)

cc_verifier_test(
    name = "extract_verify_stddef_test",
    size = "small",
    srcs = [
        "extract_verify_stddef_test.cc",
    ],
)

cc_verifier_test(
    name = "extract_verify_std_string_test",
    size = "small",
    srcs = [
        "extract_verify_std_string_test.cc",
    ],
)

cc_verifier_test(
    name = "extract_verify_std_vector_test",
    size = "small",
    srcs = [
        "extract_verify_std_vector_test.cc",
    ],
)

cc_verifier_test(
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
objc_bazel_verifier_test(
    name = "objc_lib_test",
    size = "small",
    src = "objc_lib.m",
    data = "objc_lib.xa",
    # This test can only pass on darwin, so do not include it in our presubmit.
    tags = [
        "arc-ignore",
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
cc_bazel_verifier_test(
    name = "cc_lib_test",
    size = "small",
    src = "cc_lib.cc",
    data = "cc_lib.xa",
)
