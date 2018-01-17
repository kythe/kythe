package(default_visibility = ["//visibility:public"])

package_group(
    name = "default_visibility",
    packages = ["//kythe/..."],
)

package_group(
    name = "proto_visibility",
    includes = [":default_visibility"],
    packages = ["//third_party/llvm/src"],
)

filegroup(
    name = "srcs",
    srcs = glob(
        ["**"],
        exclude = [
            "web/site/.bundle/**",
            "web/site/_vendor/**",
        ],
    ),
    visibility = ["//visibility:private"],
)
