load("//kythe/cxx/indexer/textproto/testdata:textproto_verifier_test.bzl", "textproto_verifier_test")

package(
    features = ["layering_check"],
)

textproto_verifier_test(
    name = "plugin_test",
    protos = ["example.proto"],
    textprotos = ["plugin_test.pbtxt"],
)
