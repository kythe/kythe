def empty_corpus_test(name, entries):
    """Instantiates a test that ensures all vnames in `entries` have a non-empty corpus."""
    native.sh_test(
        name = name,
        srcs = ["//kythe/go/test/tools/empty_corpus_checker:empty_corpus_test.sh"],
        args = ["$(location %s)" % entries],
        data = [
            entries,
            "//kythe/go/test/tools/empty_corpus_checker",
        ],
        env = {
            "EMPTY_CORPUS_CHECKER": "$(location //kythe/go/test/tools/empty_corpus_checker)",
        },
    )
