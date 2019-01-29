action_listener(
    name = "extract_kzip_typescript",
    extra_actions = [":extra_action"],
    mnemonics = ["TypeScriptCompile"],
    visibility = ["//visibility:public"],
)

extra_action(
    name = "extra_action",
    cmd = " ".join([
        "$(location //kythe/go/extractors/cmd/bazel:extract_kzip)",
        "--corpus=github.com/kythe/kythe",
        "--extra_action=$(EXTRA_ACTION_FILE)",
        "--include='\\.(js|json|tsx?|d\\.ts)$$'",
        "--language=typescript",
        "--output=$(output $(ACTION_ID).typescript.kzip)",
        "--rules=$(location //external:vnames_config)",
        "--scoped=true",
        "--source='\\.ts$$'",
    ]),
    data = ["//external:vnames_config"],
    out_templates = ["$(ACTION_ID).typescript.kzip"],
    tools = ["//kythe/go/extractors/cmd/bazel:extract_kzip"],
)
