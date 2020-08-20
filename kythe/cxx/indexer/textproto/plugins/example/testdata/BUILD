load("//kythe/cxx/indexer/textproto/testdata:textproto_verifier_test.bzl", "textproto_verifier_test")

package(
    features = ["layering_check"],
)

textproto_verifier_test(
    name = "cross_file_test",
    protos = ["example.proto"],
    textprotos = [
        "test1.pbtxt",
        "test2.pbtxt",
    ],
)

textproto_verifier_test(
    name = "syntax_test",
    protos = ["example.proto"],
    textprotos = [
        "syntax_test.pbtxt",
    ],
)
