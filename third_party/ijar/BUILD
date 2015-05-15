package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # Apache 2.0

cc_binary(
    name = "ijar",
    srcs = glob([
        "*.cc",
        "*.h",
    ]),
    copts = ["-Wno-unused-private-field"],
    deps = [
        "//third_party/zlib",
    ],
)
