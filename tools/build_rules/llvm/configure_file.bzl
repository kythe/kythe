def _configure_file(ctx):
    args = ctx.actions.args()
    if ctx.attr.strict:
        args.add("-strict")
    args.add("-outfile", ctx.outputs.out)
    args.add("-json", struct(**ctx.attr.defines).to_json())
    args.add(ctx.file.src)
    ctx.actions.run(
        outputs = [ctx.outputs.out],
        inputs = [ctx.file.src],
        executable = ctx.file._cmakedefines,
        arguments = [args],
        mnemonic = "ConfigureFile",
    )

configure_file = rule(
    implementation = _configure_file,
    attrs = {
        "src": attr.label(allow_single_file = True),
        "out": attr.output(mandatory = True),
        "strict": attr.bool(default = False),
        "defines": attr.string_dict(),
        "_cmakedefines": attr.label(
            default = Label("@io_kythe//tools/build_rules/llvm/cmakedefines"),
            allow_single_file = True,
            executable = True,
            cfg = "host",
        ),
    },
)
