load("//:visibility.bzl", "PUBLIC_VISIBILITY")

package(
    default_visibility = ["//kythe:default_visibility"],
    features = ["layering_check"],
)

licenses(["notice"])

cc_library(
    name = "re2_flag",
    srcs = ["re2_flag.cc"],
    hdrs = ["re2_flag.h"],
    deps = [
        "@com_google_absl//absl/flags:flag",
        "@com_google_absl//absl/strings",
        "@com_googlesource_code_re2//:re2",
    ],
)

cc_library(
    name = "scope_guard",
    hdrs = ["scope_guard.h"],
    deps = [
        "@com_google_absl//absl/base",
        "@com_google_absl//absl/log:check",
    ],
)

cc_test(
    name = "scope_guard_test",
    srcs = ["scope_guard_test.cc"],
    deps = [
        ":scope_guard",
        "//third_party:gtest",
        "//third_party:gtest_main",
    ],
)

cc_library(
    name = "json_proto",
    srcs = [
        "json_proto.cc",
    ],
    hdrs = [
        "json_proto.h",
    ],
    copts = [
        "-Wno-non-virtual-dtor",
        "-Wno-unused-variable",
        "-Wno-implicit-fallthrough",
    ],
    features = [
        "-layering_check",  # TODO: protocolbuffers/protobuf#10889
    ],
    visibility = [PUBLIC_VISIBILITY],
    deps = [
        "@com_github_tencent_rapidjson//:rapidjson",
        "@com_google_absl//absl/log",
        "@com_google_absl//absl/log:check",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/status:statusor",
        "@com_google_absl//absl/strings",
        "@com_google_protobuf//:protobuf",
        "@com_google_protobuf//src/google/protobuf/io",
    ],
)

cc_library(
    name = "kythe_uri",
    srcs = ["kythe_uri.cc"],
    hdrs = ["kythe_uri.h"],
    visibility = [PUBLIC_VISIBILITY],
    deps = [
        ":path_utils",
        ":vname_ordering",
        "//kythe/proto:storage_cc_proto",
        "@com_google_absl//absl/strings",
    ],
)

cc_library(
    name = "path_utils",
    srcs = ["path_utils.cc"],
    hdrs = ["path_utils.h"],
    visibility = [PUBLIC_VISIBILITY],
    deps = [
        ":common_status",
        "@com_google_absl//absl/container:flat_hash_map",
        "@com_google_absl//absl/log",
        "@com_google_absl//absl/memory",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/status:statusor",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/synchronization",
    ],
)

cc_test(
    name = "path_utils_test",
    srcs = ["path_utils_test.cc"],
    deps = [
        ":path_utils",
        "//third_party:gtest",
        "//third_party:gtest_main",
        "@com_google_absl//absl/log:check",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/status:statusor",
    ],
)

cc_library(
    name = "file_utils",
    srcs = ["file_utils.cc"],
    hdrs = ["file_utils.h"],
    deps = [
        "@com_google_absl//absl/log:check",
    ],
)

cc_library(
    name = "vname_ordering",
    hdrs = ["vname_ordering.h"],
    visibility = [PUBLIC_VISIBILITY],
    deps = [
        "//kythe/proto:storage_cc_proto",
    ],
)

cc_library(
    name = "lib",
    srcs = [
        "kythe_metadata_file.cc",
        "protobuf_metadata_file.cc",
    ],
    hdrs = [
        "kythe_metadata_file.h",
        "protobuf_metadata_file.h",
        "vname_ordering.h",
    ],
    copts = [
        "-Wno-non-virtual-dtor",
        "-Wno-unused-variable",
        "-Wno-implicit-fallthrough",
    ],
    features = [
        "-layering_check",  # TODO: protocolbuffers/protobuf#10889
    ],
    visibility = [PUBLIC_VISIBILITY],
    deps = [
        ":file_vname_generator",
        ":json_proto",
        "//external:zlib",
        "//kythe/cxx/common/schema:edges",
        "//kythe/proto:analysis_cc_proto",
        "//kythe/proto:metadata_cc_proto",
        "//kythe/proto:storage_cc_proto",
        "@com_github_tencent_rapidjson//:rapidjson",
        "@com_google_absl//absl/base",
        "@com_google_absl//absl/log",
        "@com_google_absl//absl/memory",
        "@com_google_absl//absl/strings",
        "@com_google_protobuf//:protobuf",
        "@com_google_protobuf//src/google/protobuf/io",
        "@com_googlesource_code_re2//:re2",
    ],
)

cc_library(
    name = "net_client",
    srcs = [
        "net_client.cc",
    ],
    hdrs = [
        "net_client.h",
    ],
    copts = [
        "-Wno-non-virtual-dtor",
        "-Wno-unused-variable",
        "-Wno-implicit-fallthrough",
    ],
    features = [
        "-layering_check",  # TODO: protocolbuffers/protobuf#10889
    ],
    deps = [
        ":json_proto",
        ":lib",
        "//external:zlib",
        "//kythe/proto:graph_cc_proto",
        "//kythe/proto:storage_cc_proto",
        "//kythe/proto:xref_cc_proto",
        "@com_github_tencent_rapidjson//:rapidjson",
        "@com_google_absl//absl/log",
        "@com_google_absl//absl/log:check",
        "@com_google_protobuf//:protobuf",
        "@com_google_protobuf//src/google/protobuf/io",
        "@se_haxx_curl//:curl",
    ],
)

cc_library(
    name = "file_vname_generator",
    srcs = ["file_vname_generator.cc"],
    hdrs = ["file_vname_generator.h"],
    deps = [
        "//kythe/proto:storage_cc_proto",
        "@com_github_tencent_rapidjson//:rapidjson",
        "@com_google_absl//absl/log",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/status:statusor",
        "@com_google_absl//absl/strings",
        "@com_googlesource_code_re2//:re2",
    ],
)

cc_test(
    name = "file_vname_generator_test",
    size = "small",
    srcs = [
        "file_vname_generator_test.cc",
    ],
    features = [
        "-layering_check",  # TODO: protocolbuffers/protobuf#10889
    ],
    deps = [
        ":file_vname_generator",
        "//third_party:gtest",
        "@com_github_inazarenko_protobuf_matchers//protobuf-matchers",
        "@com_google_protobuf//:protobuf",
        "@com_googlesource_code_re2//:re2",
    ],
)

cc_library(
    name = "json_proto_testlib",
    testonly = 1,
    srcs = [
        "json_proto_test.cc",
    ],
    copts = [
        "-Wno-non-virtual-dtor",
        "-Wno-unused-variable",
        "-Wno-implicit-fallthrough",
    ],
    features = [
        "-layering_check",  # TODO: protocolbuffers/protobuf#10889
    ],
    deps = [
        ":json_proto",
        "//kythe/proto:analysis_cc_proto",
        "//third_party:gtest",
        "@com_google_absl//absl/log:initialize",
        "@com_google_protobuf//:protobuf",
        "@com_googlesource_code_re2//:re2",
    ],
)

cc_test(
    name = "json_proto_test",
    size = "small",
    deps = [
        ":json_proto_testlib",
        "@com_google_absl//absl/log:initialize",
    ],
)

cc_library(
    name = "net_client_testlib",
    testonly = 1,
    srcs = [
        "net_client_test.cc",
    ],
    copts = [
        "-Wno-non-virtual-dtor",
        "-Wno-unused-variable",
        "-Wno-implicit-fallthrough",
    ],
    features = [
        "-layering_check",  # TODO: protocolbuffers/protobuf#10889
    ],
    deps = [
        ":json_proto",
        ":net_client",
        "//external:zlib",
        "//kythe/proto:common_cc_proto",
        "//kythe/proto:graph_cc_proto",
        "@com_google_absl//absl/flags:flag",
        "@com_google_absl//absl/flags:parse",
        "@com_google_absl//absl/log",
        "@com_google_absl//absl/log:check",
        "@com_google_absl//absl/log:initialize",
        "@com_google_absl//absl/memory",
        "@com_google_protobuf//:protobuf",
    ],
)

cc_binary(
    name = "net_client_test",
    testonly = 1,
    deps = [":net_client_testlib"],
)

cc_library(
    name = "kythe_uri_testlib",
    testonly = 1,
    srcs = ["kythe_uri_test.cc"],
    copts = [
        "-Wno-non-virtual-dtor",
        "-Wno-unused-variable",
        "-Wno-implicit-fallthrough",
    ],
    features = [
        "-layering_check",  # TODO: protocolbuffers/protobuf#10889
    ],
    deps = [
        ":kythe_uri",
        ":vname_ordering",
        "//third_party:gtest",
        "@com_github_inazarenko_protobuf_matchers//protobuf-matchers",
        "@com_google_absl//absl/log",
        "@com_google_absl//absl/log:initialize",
        "@com_google_protobuf//:protobuf",
    ],
)

cc_test(
    name = "kythe_uri_test",
    size = "small",
    deps = [":kythe_uri_testlib"],
)

cc_library(
    name = "index_writer",
    srcs = ["index_writer.cc"],
    hdrs = ["index_writer.h"],
    visibility = [PUBLIC_VISIBILITY],
    deps = [
        "//kythe/proto:analysis_cc_proto",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/status:statusor",
        "@com_google_absl//absl/strings",
    ],
)

cc_library(
    name = "index_reader",
    srcs = ["index_reader.cc"],
    hdrs = ["index_reader.h"],
    deps = [
        "//kythe/proto:analysis_cc_proto",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/status:statusor",
        "@com_google_absl//absl/strings",
    ],
)

cc_library(
    name = "kzip_encoding",
    hdrs = ["kzip_encoding.h"],
    visibility = [
        "//visibility:private",
    ],
)

cc_library(
    name = "kzip_reader",
    srcs = ["kzip_reader.cc"],
    hdrs = ["kzip_reader.h"],
    features = [
        "-layering_check",  # TODO: protocolbuffers/protobuf#10889
    ],
    visibility = [
        "//visibility:public",
    ],
    deps = [
        ":index_reader",
        ":json_proto",
        ":kzip_encoding",
        ":libzip/error",
        "//kythe/proto:analysis_cc_proto",
        "@boringssl//:crypto",
        "@com_google_absl//absl/container:flat_hash_set",
        "@com_google_absl//absl/log",
        "@com_google_absl//absl/log:check",
        "@com_google_absl//absl/memory",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/status:statusor",
        "@com_google_absl//absl/strings",
        "@com_google_protobuf//:protobuf",
        "@com_google_protobuf//src/google/protobuf/io",
        "@org_libzip//:zip",
    ],
)

cc_test(
    name = "kzip_reader_test",
    srcs = ["kzip_reader_test.cc"],
    data = [
        "//kythe/testdata/platform:empty.kzip",
        "//kythe/testdata/platform:malformed.kzip",
        "//kythe/testdata/platform:missing-pbunit.kzip",
        "//kythe/testdata/platform:missing-unit.kzip",
        "//kythe/testdata/platform:stringset.kzip",
    ],
    deps = [
        ":index_reader",
        ":kzip_reader",
        ":libzip/error",
        ":testutil",
        "//kythe/proto:go_cc_proto",  # Used in stringset.kzip.
        "//third_party:gtest",
        "//third_party:gtest_main",
        "@com_google_absl//absl/log",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/status:statusor",
        "@com_google_absl//absl/strings",
        "@org_libzip//:zip",
    ],
)

cc_library(
    name = "kzip_writer",
    srcs = ["kzip_writer.cc"],
    hdrs = ["kzip_writer.h"],
    features = [
        "-layering_check",  # TODO: protocolbuffers/protobuf#10889
    ],
    visibility = [
        "//visibility:public",
    ],
    deps = [
        ":index_writer",
        ":json_proto",
        ":kzip_encoding",
        ":libzip/error",
        "//kythe/proto:analysis_cc_proto",
        "@boringssl//:crypto",
        "@com_google_absl//absl/log",
        "@com_google_absl//absl/log:check",
        "@com_google_absl//absl/memory",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/status:statusor",
        "@com_google_absl//absl/strings",
        "@com_google_protobuf//:protobuf",
        "@org_libzip//:zip",
    ],
)

cc_test(
    name = "kzip_writer_test",
    srcs = ["kzip_writer_test.cc"],
    data = [
        "//kythe/testdata/platform:stringset.kzip",
    ],
    deps = [
        ":kzip_reader",
        ":kzip_writer",
        ":libzip/error",
        ":testutil",
        "//kythe/proto:go_cc_proto",  # Used in stringset.kzip.
        "//third_party:gtest",
        "//third_party:gtest_main",
        "@com_google_absl//absl/log",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/status:statusor",
        "@com_google_absl//absl/strings",
        "@org_libzip//:zip",
    ],
)

cc_library(
    name = "kzip_writer_c_api",
    srcs = ["kzip_writer_c_api.cc"],
    hdrs = ["kzip_writer_c_api.h"],
    features = [
        "-layering_check",  # TODO: protocolbuffers/protobuf#10889
    ],
    visibility = [
        "//visibility:public",
    ],
    deps = [
        ":index_writer",
        ":kzip_writer",
        "//kythe/proto:analysis_cc_proto",
        "@com_google_absl//absl/log",
        "@com_google_absl//absl/log:check",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/status:statusor",
        "@com_google_absl//absl/strings",
        "@com_google_protobuf//:protobuf",
    ],
)

cc_test(
    name = "kzip_writer_c_api_test",
    srcs = ["kzip_writer_c_api_test.cc"],
    data = [
        "//kythe/testdata/platform:stringset.kzip",
    ],
    deps = [
        ":kzip_reader",
        ":kzip_writer_c_api",
        ":libzip/error",
        "//kythe/proto:analysis_cc_proto",
        "//kythe/proto:go_cc_proto",  # Used in stringset.kzip.
        "//third_party:gtest",
        "//third_party:gtest_main",
        "@com_google_absl//absl/log",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/strings",
        "@org_libzip//:zip",
    ],
)

cc_library(
    # Having this library be named "status" just as that of absl status
    # affects linking this library with Rust FFI code.
    # See for details: https://github.com/bazelbuild/rules_rust/issues/385.
    name = "common_status",
    srcs = ["status.cc"],
    hdrs = ["status.h"],
    visibility = [PUBLIC_VISIBILITY],
    deps = [
        "@com_google_absl//absl/status",
    ],
)

cc_library(
    name = "libzip/error",
    srcs = ["libzip/error.cc"],
    hdrs = ["libzip/error.h"],
    deps = [
        ":common_status",
        "@com_google_absl//absl/status",
        "@org_libzip//:zip",
    ],
)

cc_library(
    name = "testutil",
    testonly = 1,
    srcs = ["testutil.cc"],
    hdrs = ["testutil.h"],
    deps = [
        "@com_google_absl//absl/log:die_if_null",
        "@com_google_absl//absl/strings",
    ],
)

cc_library(
    name = "utf8_line_index",
    srcs = ["utf8_line_index.cc"],
    hdrs = ["utf8_line_index.h"],
    visibility = [PUBLIC_VISIBILITY],
    deps = [
        "@com_google_absl//absl/algorithm:container",
        "@com_google_absl//absl/log",
        "@com_google_absl//absl/log:check",
        "@com_google_absl//absl/strings",
    ],
)

cc_test(
    name = "utf8_line_index_test",
    size = "small",
    srcs = ["utf8_line_index_test.cc"],
    deps = [
        ":utf8_line_index",
        "//third_party:gtest",
        "//third_party:gtest_main",
        "@com_google_absl//absl/log",
        "@com_google_absl//absl/log:check",
        "@com_google_absl//absl/strings",
    ],
)

cc_library(
    name = "init",
    srcs = ["init.cc"],
    hdrs = ["init.h"],
    local_defines = ["KYTHE_OVERRIDE_ASSERT_FAIL"],
    deps = [
        "@com_google_absl//absl/debugging:failure_signal_handler",
        "@com_google_absl//absl/debugging:symbolize",
        "@com_google_absl//absl/log",
        "@com_google_absl//absl/log:flags",
        "@com_google_absl//absl/log:initialize",
    ],
)

cc_library(
    name = "regex",
    srcs = ["regex.cc"],
    hdrs = ["regex.h"],
    deps = [
        "@com_google_absl//absl/log",
        "@com_google_absl//absl/log:check",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/status:statusor",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/types:span",
        "@com_googlesource_code_re2//:re2",
    ],
)

cc_test(
    name = "regex_test",
    srcs = ["regex_test.cc"],
    deps = [
        ":regex",
        "//third_party:gtest",
        "//third_party:gtest_main",
        "@com_google_absl//absl/log",
        "@com_googlesource_code_re2//:re2",
    ],
)

cc_library(
    name = "sha256_hasher",
    srcs = ["sha256_hasher.cc"],
    hdrs = ["sha256_hasher.h"],
    deps = [
        "@boringssl//:crypto",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/types:span",
    ],
)
