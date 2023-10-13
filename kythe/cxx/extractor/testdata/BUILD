load(
    "//tools/build_rules/verifier_test:cc_indexer_test.bzl",
    "cc_bazel_extractor_test",
    "cc_extractor_test",
    "objc_bazel_extractor_test",
)
load("//tools:build_rules/testing.bzl", "shell_tool_test")

package(
    default_visibility = ["//kythe:default_visibility"],
    features = ["layering_check"],
)

filegroup(
    name = "skip",
    srcs = ["skip_functions.sh"],
)

cc_test(
    name = "root_directory_test",
    srcs = ["root_directory_test.cc"],
    data = [
        "altroot/altpath/file.cc",
    ],
    deps = [
        "//kythe/cxx/common:path_utils",
        "//kythe/cxx/extractor:testlib",
        "//kythe/proto:analysis_cc_proto",
        "//third_party:gmock_main",
        "//third_party:gtest",
        "@com_github_inazarenko_protobuf_matchers//protobuf-matchers",
        "@com_google_absl//absl/strings",
    ],
)

cc_test(
    name = "alternate_platform_test",
    srcs = ["alternate_platform_test.cc"],
    data = ["arm.cc"],
    deps = [
        "//kythe/cxx/extractor:testlib",
        "//kythe/proto:analysis_cc_proto",
        "//third_party:gmock_main",
        "//third_party:gtest",
        "@com_github_inazarenko_protobuf_matchers//protobuf-matchers",
        "@com_google_absl//absl/strings",
    ],
)

cc_test(
    name = "main_source_file_env_dep_test",
    srcs = ["main_source_file_env_dep_test.cc"],
    data = [
        "main_source_file_env_dep.cc",
    ],
    deps = [
        "//kythe/cxx/extractor:testlib",
        "//kythe/proto:analysis_cc_proto",
        "//third_party:gmock_main",
        "//third_party:gtest",
        "@com_github_inazarenko_protobuf_matchers//protobuf-matchers",
        "@com_google_absl//absl/algorithm:container",
        "@com_google_absl//absl/strings",
    ],
)

cc_test(
    name = "main_source_file_no_env_dep_test",
    srcs = ["main_source_file_no_env_dep_test.cc"],
    data = [
        "main_source_file_no_env_dep.cc",
    ],
    deps = [
        "//kythe/cxx/extractor:testlib",
        "//kythe/proto:analysis_cc_proto",
        "//third_party:gmock_main",
        "//third_party:gtest",
        "@com_github_inazarenko_protobuf_matchers//protobuf-matchers",
        "@com_google_absl//absl/algorithm:container",
        "@com_google_absl//absl/strings",
    ],
)

cc_test(
    name = "header_only_module_test",
    srcs = ["header_only_module_test.cc"],
    data = [
        "header_only_module.cppmap",
        "header_only_module.h",
    ],
    deps = [
        "//kythe/cxx/extractor:testlib",
        "//kythe/proto:analysis_cc_proto",
        "//third_party:gmock_main",
        "//third_party:gtest",
        "@com_github_inazarenko_protobuf_matchers//protobuf-matchers",
        "@com_google_absl//absl/algorithm:container",
        "@com_google_absl//absl/strings",
    ],
)

cc_test(
    name = "extract_transcript_test",
    srcs = ["extract_transcript_test.cc"],
    data = [
        "transcript_a.h",
        "transcript_b.h",
        "transcript_main.cc",
    ],
    deps = [
        "//kythe/cxx/extractor:testlib",
        "//kythe/proto:analysis_cc_proto",
        "//third_party:gmock_main",
        "//third_party:gtest",
        "@com_github_inazarenko_protobuf_matchers//protobuf-matchers",
        "@com_google_absl//absl/strings",
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
    features = [
        "-layering_check",  # TODO: protocolbuffers/protobuf#10889
    ],
    deps = [
        "//kythe/cxx/extractor:lib",
        "//kythe/cxx/extractor:supported_language",
        "//third_party:gmock_main",
        "//third_party:gtest",
        "@com_google_absl//absl/log:check",
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

cc_test(
    name = "has_include_test",
    srcs = ["has_include_test.cc"],
    data = [
        "has_include.cc",
        "has_include.h",
    ],
    deps = [
        "//kythe/cxx/extractor:testlib",
        "//kythe/proto:analysis_cc_proto",
        "//third_party:gmock_main",
        "//third_party:gtest",
        "@com_github_inazarenko_protobuf_matchers//protobuf-matchers",
        "@com_google_absl//absl/strings",
    ],
)

cc_test(
    name = "indirect_has_include_test",
    srcs = ["indirect_has_include_test.cc"],
    data = [
        "has_include.h",
        "indirect_has_include.cc",
        "indirect_has_include.h",
    ],
    deps = [
        "//kythe/cxx/extractor:testlib",
        "//kythe/proto:analysis_cc_proto",
        "//third_party:gmock_main",
        "//third_party:gtest",
        "@com_github_inazarenko_protobuf_matchers//protobuf-matchers",
        "@com_google_absl//absl/strings",
    ],
)

cc_test(
    name = "metadata_test",
    srcs = ["metadata_test.cc"],
    data = [
        "metadata.cc",
        "metadata.cc.meta",
    ],
    deps = [
        "//kythe/cxx/extractor:testlib",
        "//kythe/proto:analysis_cc_proto",
        "//third_party:gmock_main",
        "//third_party:gtest",
        "@com_github_inazarenko_protobuf_matchers//protobuf-matchers",
        "@com_google_absl//absl/strings",
    ],
)

cc_test(
    name = "modules_test",
    srcs = ["modules_test.cc"],
    data = [
        "modfoo.h",
        "modfoo.modulemap",
        "modules.cc",
    ],
    deps = [
        "//kythe/cxx/extractor:testlib",
        "//kythe/proto:analysis_cc_proto",
        "//third_party:gmock_main",
        "//third_party:gtest",
        "@com_github_inazarenko_protobuf_matchers//protobuf-matchers",
        "@com_google_absl//absl/algorithm:container",
        "@com_google_absl//absl/strings",
    ],
)

cc_test(
    name = "build_config_test",
    srcs = ["build_config_test.cc"],
    data = [
        "build_config.cc",
    ],
    features = [
        "-layering_check",  # TODO: protocolbuffers/protobuf#10889
    ],
    deps = [
        "//kythe/cxx/extractor:testlib",
        "//kythe/proto:analysis_cc_proto",
        "//kythe/proto:buildinfo_cc_proto",
        "//third_party:gmock_main",
        "//third_party:gtest",
        "@com_github_inazarenko_protobuf_matchers//protobuf-matchers",
        "@com_google_absl//absl/log",
        "@com_google_absl//absl/strings",
        "@com_google_protobuf//:protobuf",
    ],
)

cc_test(
    name = "installed_dir_test",
    srcs = ["installed_dir_test.cc"],
    data = [
        # Clang resolves the include directory relative to `bin`,
        # so the former needs to exist.
        "bin/clang",
        "include/c++/v1/dummy",
        "installed_dir.cc",
    ],
    deps = [
        "//kythe/cxx/extractor:testlib",
        "//kythe/proto:analysis_cc_proto",
        "//third_party:gmock_main",
        "//third_party:gtest",
        "@com_github_inazarenko_protobuf_matchers//protobuf-matchers",
        "@com_google_absl//absl/strings",
    ],
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
    target_compatible_with = [
        "@platforms//os:osx",
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
    # This test no longer passes, even on OSX, so do not include it in our presubmit.
    tags = [
        "arc-ignore",
        "broken",
        "manual",
    ],
    target_compatible_with = [
        "@platforms//os:osx",
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
