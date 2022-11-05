load("@bazel_skylib//lib:selects.bzl", "selects")

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
    )
]
