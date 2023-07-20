load("@bazel_skylib//lib:selects.bzl", "selects")
load("@rules_java//toolchains:default_java_toolchain.bzl", "default_java_toolchain")

package(default_visibility = ["//visibility:public"])

# We need to have consistent JDK dependencies, which Bazel often doesn't provide out-of-the-box.
# In particular, when a target requires a particular JDK version it generally depends on
# internal class peculiar to that JDK, thus all of `source_version`, `target_version`,
# and (critically) `java_runtime` must match. Some of the upstream toolchains use a `java_runtime`
# which differs from the source and target version.
[
    default_java_toolchain(
        name = "toolchain_jdk_{version}".format(version = version),
        configuration = dict(),
        java_runtime = "@remotejdk{version}_linux//:jdk".format(version = version),
        source_version = version,
        target_version = version,
    )
    for version in ("11", "17", "19", "20", "21")
]

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
        "20": [
            "remotejdk_20",
            "20",
        ],
        "21": [
            "remotejdk_21",
            "21",
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
        "20",
        "21",
    )
]
