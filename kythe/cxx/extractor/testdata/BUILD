load("//tools:build_rules/kythe.bzl", "cc_verifier_test")

package(default_visibility = ["//kythe:default_visibility"])

sh_test(
    name = "root_directory",
    size = "small",
    srcs = [
        "test_root_directory.sh",
    ],
    data = [
        "altroot/altpath/file.cc",
        "altroot_altpath_file.UNIT",
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
        "test_common.sh",
        "transcript_a.h",
        "transcript_b.h",
        "transcript_main.UNIT",
        "transcript_main.cc",
        "//kythe/cxx/extractor:cxx_extractor",
        "//kythe/cxx/tools:kindex_tool",
    ],
)

sh_test(
    name = "claim_pragma",
    size = "small",
    srcs = [
        "test_claim_pragma.sh",
    ],
    data = [
        "claim_a.h",
        "claim_b.h",
        "claim_main.UNIT",
        "claim_main.cc",
        "test_common.sh",
        "//kythe/cxx/extractor:cxx_extractor",
        "//kythe/cxx/tools:kindex_tool",
    ],
)

sh_test(
    name = "stdin_names",
    size = "small",
    srcs = [
        "test_stdin_names.sh",
    ],
    data = [
        "stdin.vnames",
        "test_common.sh",
        "test_stdin_names_verify.cc",
        "//kythe/cxx/extractor:cxx_extractor",
        "//kythe/cxx/indexer/cxx:indexer",
        "//kythe/cxx/verifier",
        "//kythe/go/platform/tools:indexpack",
    ],
)

sh_test(
    name = "index_pack",
    size = "small",
    srcs = [
        "test_index_pack.sh",
    ],
    data = [
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
