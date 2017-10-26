package(default_visibility = ["//visibility:public"])

environment(name = "linux")

environment(
    name = "k8",
    fulfills = [":linux"],
)

environment(name = "darwin")

environment(
    name = "all",
    fulfills = [
        ":linux",
        ":darwin",
        ":k8",
    ],
)

environment_group(
    name = "cpu",
    defaults = [":all"],
    environments = [
        ":k8",
        ":linux",
        ":darwin",
        ":all",
    ],
)
