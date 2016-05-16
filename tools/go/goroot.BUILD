package(default_visibility = ["//visibility:public"])

config_setting(
    name = "debug",
    values = {"compilation_mode": "dbg"},
)

filegroup(
    name = "default-goroot",
    srcs = [
        ":gotool",
        ":src",
        ":pkg",
    ] + select({
        # Add race-enabled archives only for -c dbg builds
        ":debug": [":pkg_race"],
        "//conditions:default": [],
    }),
)

filegroup(
    name = "gotool",
    srcs = ["bin/go"],
)

filegroup(
    name = "src",
    srcs = glob([
        "src/**/*.go",
        "src/**/*.s",
        "src/**/*.h",
    ]),
)

filegroup(
    name = "pkg",
    srcs = glob(
        [
            "pkg/**/*.a",
            "pkg/include/**/*.h",
            "pkg/tool/*/*",
        ],
        exclude = ["pkg/*_race/**"],
    ),
)

filegroup(
    name = "pkg_race",
    srcs = glob(["pkg/*_race/**/*.a"]),
)
