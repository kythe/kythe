"""Build rules used to define toolchains for external tools."""

def _external_tools_toolchain_impl(ctx):
    return [
        platform_common.ToolchainInfo(
            asciidoc = ctx.attr.asciidoc,
            dot = ctx.attr.dot,
            python = ctx.attr.python,
            cat = ctx.attr.cat,
            path = ctx.attr.path,
        ),
    ]

external_tools_toolchain = rule(
    implementation = _external_tools_toolchain_impl,
    attrs = {
        "asciidoc": attr.string(),
        "dot": attr.string(),
        "python": attr.string(),
        "cat": attr.string(),
        "path": attr.string(),
    },
)
