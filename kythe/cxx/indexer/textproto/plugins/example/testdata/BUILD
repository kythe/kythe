load("//kythe/cxx/indexer/textproto/testdata:textproto_verifier_test.bzl", "textproto_verifier_test")

package(features = ["layering_check"])

textproto_verifier_test(
    name = "cross_file_test",
    indexer_opts = ["--enable_example_plugin"],
    protos = [":example_proto"],
    textprotos = [
        "test1.pbtxt",
        "test2.pbtxt",
    ],
)

textproto_verifier_test(
    name = "syntax_test",
    indexer_opts = ["--enable_example_plugin"],
    protos = [":example_proto"],
    textprotos = [
        "syntax_test.pbtxt",
    ],
)

proto_library(
    name = "example_proto",
    srcs = ["example.proto"],
)
