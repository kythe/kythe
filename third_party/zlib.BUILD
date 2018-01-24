package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # BSD/MIT-like license (for zlib)

_ZLIB_HEADERS = [
    "crc32.h",
    "deflate.h",
    "gzguts.h",
    "inffast.h",
    "inffixed.h",
    "inflate.h",
    "inftrees.h",
    "trees.h",
    "zconf.h",
    "zlib.h",
    "zutil.h",
]

# In order to limit the damage from the `includes` propagation
# via `:zlib`, copy the public headers to a subdirectory and
# expose those.
genrule(
    name = "copy_public_headers",
    srcs = _ZLIB_HEADERS,
    outs = ["zlib/include/" + hdr for hdr in _ZLIB_HEADERS],
    cmd = "cp $(SRCS) $D",
    visibility = ["//visibility:private"],
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
        # Include the un-prefixed headers in srcs to work
        # around the fact that zlib isn't consistent in its
        # choice of <> or "" delimiter when including itself.
    ] + _ZLIB_HEADERS,
    hdrs = glob(["zlib/include/*"]),
    copts = [
        "-Wno-unused-variable",
        "-Wno-implicit-function-declaration",
    ],
    includes = ["zlib/include/"],
)
