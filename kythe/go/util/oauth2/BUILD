load("//tools/build_rules/go", "go_library")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "oauth2",
    srcs = [
        "oauth2.go",
    ],
    deps = [
        "//third_party/go:cloud",
        "//third_party/go:context",
        "//third_party/go:oauth2",
        "//third_party/go:oauth2_google",
    ],
)
