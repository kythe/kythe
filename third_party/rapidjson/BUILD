licenses(["notice"])

package(default_visibility = ["//visibility:public"])

cc_library(
    name = "rapidjson",
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
