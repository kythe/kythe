package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # BSD/MIT-like license (for zlib)

filegroup(
    name = "license",
    srcs = ["LICENSE"],
)

cc_library(
    name = "zlib",
    srcs = [
        "adler32.c",
        "compress.c",
        "crc32.c",
        "deflate.c",
        "gzclose.c",
        "gzlib.c",
        "gzread.c",
        "gzwrite.c",
        "infback.c",
        "inffast.c",
        "inflate.c",
        "inftrees.c",
        "trees.c",
        "uncompr.c",
        "zutil.c",
    ],
    hdrs = [
        "zlib/include/crc32.h",
        "zlib/include/deflate.h",
        "zlib/include/gzguts.h",
        "zlib/include/inffast.h",
        "zlib/include/inffixed.h",
        "zlib/include/inflate.h",
        "zlib/include/inftrees.h",
        "zlib/include/trees.h",
        "zlib/include/zconf.h",
        "zlib/include/zlib.h",
        "zlib/include/zutil.h",
    ],
    copts = [
        "-Wno-unused-variable",
        "-Wno-implicit-function-declaration",
    ],
    includes = ["zlib/include/"],
)
