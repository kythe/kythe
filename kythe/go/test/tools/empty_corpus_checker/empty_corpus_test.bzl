def empty_corpus_test(name, entries, allowed_corpora = []):
    """Instantiates a test that ensures all vnames in `entries` have a non-empty corpus."""

    args = ["$(location %s)" % entries]
    if len(allowed_corpora) > 0:
        args.append("--allowed_corpora=" + ",".join(allowed_corpora))

    native.sh_test(
        name = name,
        srcs = ["//kythe/go/test/tools/empty_corpus_checker:empty_corpus_test.sh"],
        args = args,
        data = [
            entries,
            "//kythe/go/test/tools/empty_corpus_checker",
        ],
        env = {
            "EMPTY_CORPUS_CHECKER": "$(location //kythe/go/test/tools/empty_corpus_checker)",
        },
    )
