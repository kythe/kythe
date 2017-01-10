package(default_visibility = ["//kythe:default_visibility"])

# TODO(schroederc): test is timing out on HEAD after update of Go/Java GRPC libraries
# sh_test(
#     name = "analyzer_driver_test",
#     size = "small",
#     srcs = ["testdata/analyzer_driver_test.sh"],
#     data = [
#         "//kythe/go/platform/tools/analyzer_driver",
#         "//kythe/go/platform/tools/entrystream",
#         "//kythe/java/com/google/devtools/kythe/analyzers/java:indexer_server",
#         "//kythe/testdata:test.kindex",
#     ],
#     tags = [
#         "flaky",  # flaky due to port picking
#         "local",
#     ],
# )

sh_test(
    name = "indexpack_test",
    size = "small",
    srcs = ["indexpack_test.sh"],
    data = [
        "//kythe/go/platform/tools/indexpack",
        "//kythe/go/platform/tools/viewindex",
        "//kythe/testdata:test.kindex",
        "//third_party/jq",
    ],
)
