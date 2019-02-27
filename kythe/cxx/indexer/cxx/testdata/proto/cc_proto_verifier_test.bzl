load("//tools/build_rules/verifier_test:cc_indexer_test.bzl", "cc_extract_kzip", "cc_index", "cc_kythe_proto_library")
load("//tools/build_rules/verifier_test:verifier_test.bzl", "index_compilation", "verifier_test")
load(
    "@io_kythe_lang_proto//kythe/cxx/indexer/proto/testdata:proto_verifier_test.bzl",
    "proto_extract_kzip",
)

def cc_proto_verifier_test(
        name,
        srcs,
        size = "small",
        proto_srcs = [],
        verifier_opts = [
            "--ignore_dups",
            # Else the verifier chokes on the inconsistent marked source from the protobuf headers.
            "--convert_marked_source",
        ]):
    """Verify cross-language references between C++ and Proto.

    Args:
      name: Name of the test.
      size: Size of the test.
      srcs: The compilation's C++ source files; each file's verifier goals will be checked
      proto_srcs: The compilation's proto source files; each file's verifier goals will be checked
      verifier_opts: List of options passed to the verifier tool

    Returns: the label of the test.
    """
    proto_kzip = _invoke(
        proto_extract_kzip,
        name = name + "_proto_kzip",
        srcs = proto_srcs,
    )

    proto_entries = _invoke(
        index_compilation,
        name = name + "_proto_entries",
        indexer = "@io_kythe_lang_proto//kythe/cxx/indexer/proto:indexer",
        opts = ["--index_file"],
        deps = [proto_kzip],
    )

    proto_lib = _invoke(
        cc_kythe_proto_library,
        name = name + "_cc_proto",
        deps = ["//kythe/javatests/com/google/devtools/kythe/analyzers/java/testdata/pkg/proto:testdata_proto"],
    )

    cc_kzip = _invoke(
        cc_extract_kzip,
        name = name + "_cc_kzip",
        srcs = srcs,
        deps = [proto_lib],
    )

    cc_entries = _invoke(
        cc_index,
        name = name + "_cc_entries",
        srcs = [cc_kzip],
        opts = [
            # Avoid emitting some nodes with conflicting facts.
            "--experimental_index_lite",
            # Try to reduce the graph size to make life easier for the verifier.
            "--test_claim",
            "--noindex_template_instantiations",
            "--experimental_drop_instantiation_independent_data",
            "--noemit_anchors_on_builtins",
        ],
    )

    return _invoke(
        verifier_test,
        name = name,
        srcs = srcs + proto_srcs,
        opts = verifier_opts,
        size = size,
        deps = [
            cc_entries,
            proto_entries,
        ],
    )

def _invoke(rulefn, name, **kwargs):
    """Invoke rulefn with name and kwargs, returning the label of the rule."""
    rulefn(name = name, **kwargs)
    return "//{}:{}".format(native.package_name(), name)
