load("@bazel_skylib//lib:selects.bzl", "selects")
load("@rules_java//toolchains:default_java_toolchain.bzl", "default_java_toolchain")

package(default_visibility = ["//visibility:public"])

# Bazel does not yet ship with a JDK-19 compatible JDK, so
# fetch and install our own.
default_java_toolchain(
    name = "toolchain_jdk_19",
    configuration = dict(),
    java_runtime = "@remotejdk19_linux//:jdk",
    source_version = "19",
    target_version = "19",
)

config_setting(
    name = "runtime_version_default",
    values = {
        "java_runtime_version": "",
    },
)

[
    selects.config_setting_group(
        name = "runtime_version_{version}".format(version = v),
        match_any = [
            (
                config_setting(
                    name = "runtime_version_{version}_{value}".format(
                        value = value,
                        version = v,
                    ),
                    values = {
                        "java_runtime_version": value,
                    },
                    visibility = ["//visibility:private"],
                ),
                ":runtime_version_{version}_{value}".format(
                    value = value,
                    version = v,
                ),
            )[1]
            for value in values
        ],
    )
    for v, values in {
        "11": [
            "remotejdk_11",
            "11",
            "rbe_jdk",
        ],
        "17": [
            "remotejdk_17",
            "17",
        ],
        "19": [
            "remotejdk_19",
            "19",
        ],
    }.items()
]

config_setting(
    name = "language_version_default",
    values = {
        "java_language_version": "",
    },
)

[
    config_setting(
        name = "language_version_{}".format(v),
        values = {"java_language_version": v},
    )
    for v in (
        "11",
        "17",
        "19",
    )
]
