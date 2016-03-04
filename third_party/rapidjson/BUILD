licenses(["notice"])

package(default_visibility = ["//visibility:public"])

filegroup(
    name = "license",
    srcs = ["license.txt"],
)

cc_library(
    name = "rapidjson",
    srcs = [
        # To satisfy the OSX toolchain
        # See https://github.com/google/bazel/issues/61
        "empty.cc",
    ],
    hdrs = [
        "include/rapidjson/allocators.h",
        "include/rapidjson/document.h",
        "include/rapidjson/encodedstream.h",
        "include/rapidjson/encodings.h",
        "include/rapidjson/error/en.h",
        "include/rapidjson/error/error.h",
        "include/rapidjson/filereadstream.h",
        "include/rapidjson/filestream.h",
        "include/rapidjson/filewritestream.h",
        "include/rapidjson/internal/dtoa.h",
        "include/rapidjson/internal/itoa.h",
        "include/rapidjson/internal/meta.h",
        "include/rapidjson/internal/pow10.h",
        "include/rapidjson/internal/stack.h",
        "include/rapidjson/internal/strfunc.h",
        "include/rapidjson/memorybuffer.h",
        "include/rapidjson/memorystream.h",
        "include/rapidjson/msinttypes/inttypes.h",
        "include/rapidjson/msinttypes/stdint.h",
        "include/rapidjson/prettywriter.h",
        "include/rapidjson/rapidjson.h",
        "include/rapidjson/reader.h",
        "include/rapidjson/stringbuffer.h",
        "include/rapidjson/writer.h",
    ],
    copts = [
        "-Wno-non-virtual-dtor",
        "-Wno-unused-variable",
        "-Wno-implicit-fallthrough",
    ],
    includes = [
        "include",
    ],
)

cc_library(
    name = "rapidjson_unit_test_lib",
    srcs = [
        "test/unittest/unittest.cc",
    ],
    hdrs = [
        "test/unittest/unittest.h",
    ],
    copts = [
        "-Wno-non-virtual-dtor",
        "-Wno-unused-variable",
        "-Wno-implicit-fallthrough",
    ],
    deps = [
        ":rapidjson",
        "//external:googletest",
    ],
)

cc_library(
    name = "rapidjson_writer_test_lib",
    srcs = [
        "test/unittest/writertest.cc",
    ],
    hdrs = [
        "test/unittest/unittest.h",
    ],
    copts = [
        "-Wno-non-virtual-dtor",
        "-Wno-unused-variable",
        "-Wno-implicit-fallthrough",
    ],
    deps = [
        ":rapidjson",
        "//external:googletest",
    ],
)

cc_test(
    name = "rapidjson_writer_test",
    deps = [
        ":rapidjson_unit_test_lib",
        ":rapidjson_writer_test_lib",
    ],
)

cc_library(
    name = "rapidjson_value_test_lib",
    srcs = [
        "test/unittest/valuetest.cc",
    ],
    hdrs = [
        "test/unittest/unittest.h",
    ],
    copts = [
        "-Wno-non-virtual-dtor",
        "-Wno-unused-variable",
        "-Wno-implicit-fallthrough",
    ],
    deps = [
        ":rapidjson",
        "//external:googletest",
    ],
)

cc_test(
    name = "rapidjson_value_test",
    deps = [
        ":rapidjson_unit_test_lib",
        ":rapidjson_value_test_lib",
    ],
)

cc_library(
    name = "rapidjson_string_buffer_test_lib",
    srcs = [
        "test/unittest/stringbuffertest.cc",
    ],
    hdrs = [
        "test/unittest/unittest.h",
    ],
    copts = [
        "-Wno-non-virtual-dtor",
        "-Wno-unused-variable",
        "-Wno-implicit-fallthrough",
    ],
    deps = [
        ":rapidjson",
        "//external:googletest",
    ],
)

cc_test(
    name = "rapidjson_string_buffer_test",
    deps = [
        ":rapidjson_string_buffer_test_lib",
        ":rapidjson_unit_test_lib",
    ],
)

cc_library(
    name = "rapidjson_reader_test_lib",
    srcs = [
        "test/unittest/readertest.cc",
    ],
    hdrs = [
        "test/unittest/unittest.h",
    ],
    copts = [
        "-Wno-non-virtual-dtor",
        "-Wno-unused-variable",
        "-Wno-implicit-fallthrough",
    ],
    deps = [
        ":rapidjson",
        "//external:googletest",
    ],
)

cc_test(
    name = "rapidjson_reader_test",
    deps = [
        ":rapidjson_reader_test_lib",
        ":rapidjson_unit_test_lib",
    ],
)

cc_library(
    name = "rapidjson_json_checker_test_lib",
    srcs = [
        "test/unittest/jsoncheckertest.cc",
    ],
    hdrs = [
        "test/unittest/unittest.h",
    ],
    copts = [
        "-Wno-non-virtual-dtor",
        "-Wno-unused-variable",
        "-Wno-implicit-fallthrough",
    ],
    deps = [
        ":rapidjson",
        "//external:googletest",
    ],
)

cc_test(
    name = "rapidjson_json_checker_test",
    data = glob([
        "data/*",
        "encodings/*",
        "jsonchecker/*",
    ]),
    deps = [
        ":rapidjson_json_checker_test_lib",
        ":rapidjson_unit_test_lib",
    ],
)

cc_library(
    name = "rapidjson_file_stream_test_lib",
    srcs = [
        "test/unittest/filestreamtest.cc",
    ],
    hdrs = [
        "test/unittest/unittest.h",
    ],
    copts = [
        "-Wno-non-virtual-dtor",
        "-Wno-unused-variable",
        "-Wno-implicit-fallthrough",
    ],
    deps = [
        ":rapidjson",
        "//external:googletest",
    ],
)

cc_test(
    name = "rapidjson_file_stream_test",
    data = glob([
        "data/*",
        "encodings/*",
    ]),
    deps = [
        ":rapidjson_file_stream_test_lib",
        ":rapidjson_unit_test_lib",
    ],
)

cc_library(
    name = "rapidjson_encodings_test_lib",
    srcs = [
        "test/unittest/encodingstest.cc",
    ],
    hdrs = [
        "test/unittest/unittest.h",
    ],
    copts = [
        "-Wno-non-virtual-dtor",
        "-Wno-unused-variable",
        "-Wno-implicit-fallthrough",
    ],
    deps = [
        ":rapidjson",
        "//external:googletest",
    ],
)

cc_test(
    name = "rapidjson_encodings_test",
    deps = [
        ":rapidjson_encodings_test_lib",
        ":rapidjson_unit_test_lib",
    ],
)

cc_library(
    name = "rapidjson_encoded_stream_test_lib",
    srcs = [
        "test/unittest/encodedstreamtest.cc",
    ],
    hdrs = [
        "test/unittest/unittest.h",
    ],
    copts = [
        "-Wno-non-virtual-dtor",
        "-Wno-unused-variable",
        "-Wno-implicit-fallthrough",
    ],
    deps = [
        ":rapidjson",
        "//external:googletest",
    ],
)

cc_test(
    name = "rapidjson_encoded_stream_test",
    data = glob([
        "data/*",
        "encodings/*",
    ]),
    deps = [
        ":rapidjson_encoded_stream_test_lib",
        ":rapidjson_unit_test_lib",
    ],
)

cc_library(
    name = "rapidjson_document_test_lib",
    srcs = [
        "test/unittest/documenttest.cc",
    ],
    hdrs = [
        "test/unittest/unittest.h",
    ],
    copts = [
        "-Wno-non-virtual-dtor",
        "-Wno-unused-variable",
        "-Wno-implicit-fallthrough",
    ],
    deps = [
        ":rapidjson",
        "//external:googletest",
    ],
)

cc_test(
    name = "rapidjson_document_test",
    data = glob([
        "data/*",
        "encodings/*",
    ]),
    deps = [
        ":rapidjson_document_test_lib",
        ":rapidjson_unit_test_lib",
    ],
)

cc_library(
    name = "rapidjson_perf_test_lib",
    srcs = [
        "test/perftest/perftest.cc",
    ],
    hdrs = [
        "test/perftest/perftest.h",
    ],
    copts = [
        "-Wno-non-virtual-dtor",
        "-Wno-unused-variable",
        "-Wno-implicit-fallthrough",
    ],
    deps = [
        ":rapidjson",
        "//external:googletest",
    ],
)

cc_library(
    name = "rapidjson_misc_perf_test_lib",
    srcs = [
        "test/perftest/misctest.cc",
    ],
    hdrs = [
        "test/perftest/perftest.h",
    ],
    copts = [
        "-Wno-non-virtual-dtor",
        "-Wno-unused-variable",
        "-Wno-implicit-fallthrough",
    ],
    deps = [
        ":rapidjson",
        "//external:googletest",
    ],
)

#cc_test(
#    name = "rapidjson_misc_perf_test",
#    deps = [
#        ":rapidjson_misc_perf_test_lib",
#        ":rapidjson_perf_test_lib",
#    ],
#    data = glob(["data/*", "encodings/*"]),
#)
cc_library(
    name = "rapidjson_rapid_json_perf_test_lib",
    srcs = [
        "test/perftest/rapidjsontest.cc",
    ],
    hdrs = [
        "test/perftest/perftest.h",
    ],
    copts = [
        "-Wno-non-virtual-dtor",
        "-Wno-unused-variable",
        "-Wno-implicit-fallthrough",
    ],
    deps = [
        ":rapidjson",
        "//external:googletest",
    ],
)

#cc_test(
#    name = "rapidjson_rapid_json_perf_test",
#    deps = [
#        ":rapidjson_perf_test_lib",
#        ":rapidjson_rapid_json_perf_test_lib",
#    ],
#    data = glob(["data/*"]),
#)
#cc_library(
#    name = "rapidjson_platform_perf_test_lib",
#    srcs = [
#        "test/perftest/platformtest.cc",
#    ],
#    hdrs = [
#        "test/perftest/perftest.h",
#    ],
#    copts = [
#        "-Wno-non-virtual-dtor",
#        "-Wno-unused-variable",
#        "-Wno-implicit-fallthrough",
#    ],
#    deps = [
#        ":rapidjson",
#        "//external:googletest",
#    ],
#)
#cc_test(
#    name = "rapidjson_platform_perf_test",
#    deps = [
#        ":rapidjson_perf_test_lib",
#        ":rapidjson_platform_perf_test_lib",
#    ],
#    data = glob(["data/*", "encodings/*"]),
#)
