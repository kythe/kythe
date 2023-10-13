# Files beneath here are automatically generated.
# gazelle:exclude *
# Targets related to platform configs.
sh_binary(
    name = "rebuild",
    srcs = ["rebuild.sh"],
    data = [
        "@bazel_toolchains_rbe_gen_config_linux_amd64//file",
    ],
    env = {
        "RBE_GEN_CONFIG": "$(execpath @bazel_toolchains_rbe_gen_config_linux_amd64//file)",
        "OUTPUT_CONFIG_PATH": "tools/platforms/configs/rbe_default",
    },
    tags = [
        "local",
        "manual",
    ],
)
