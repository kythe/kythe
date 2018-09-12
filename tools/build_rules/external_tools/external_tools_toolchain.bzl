"""Build rules used to define toolchains for external tools."""

def _external_tools_toolchain_impl(ctx):
    return [
        platform_common.ToolchainInfo(
            asciidoc = ctx.attr.asciidoc,
            path = ctx.attr.path,
        ),
    ]

external_tools_toolchain = rule(
    implementation = _external_tools_toolchain_impl,
    attrs = {
        "asciidoc": attr.string(),
        "path": attr.string(),
    },
)
