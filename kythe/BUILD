package(default_visibility = ["//visibility:public"])

package_group(
    name = "default_visibility",
    packages = [
        "//kythe/...",
    ],
)

filegroup(
    name = "srcs",
    srcs = glob(["**"], exclude=["web/site/.bundle/**", "web/site/_vendor/**"]),
    visibility = ["//visibility:private"],
)
