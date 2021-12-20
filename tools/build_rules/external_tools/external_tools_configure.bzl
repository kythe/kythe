"""Repository rules for automagically configuring the host toolchain."""

_BUILD_TEMPLATE = """
package(default_visibility=["//visibility:public"])

load("@io_kythe//tools/build_rules/external_tools:external_tools_toolchain.bzl", "external_tools_toolchain")

external_tools_toolchain(
  name = "host_toolchain_impl",
  asciidoc = "{asciidoc}",
  path = "{path}"
)

toolchain(
    name = "host_toolchain",
    toolchain = ":host_toolchain_impl",
    toolchain_type = "@io_kythe//tools/build_rules/external_tools:external_tools_toolchain_type",
)
"""

def _external_toolchain_autoconf_impl(repository_ctx):
    if repository_ctx.os.environ.get("KYTHE_DO_NOT_DETECT_BAZEL_TOOLCHAINS", "0") == "1":
        repository_ctx.file("BUILD", "# Toolchain detection disabled by KYTHE_DO_NOT_DETECT_BAZEL_TOOLCHAINS")
        return
    asciidoc = repository_ctx.which("asciidoc")
    if asciidoc == None:
        fail("Unable to find 'asciidoc' executable on path.")

    # These are the tools that the doc/schema generation need beyond the
    # explicit call to asciidoc.
    tools = [
        "awk",
        "bash",
        "cat",
        "cut",
        "dot",
        "env",
        "find",
        "grep",
        "mkdir",
        "mktemp",
        "mv",
        "python3",
        "readlink",
        "rm",
        "sed",
        "source-highlight",
        "tee",
        "touch",
        "zip",
    ]
    for tool in tools:
        symlink_command(repository_ctx, tool)

    repository_ctx.file("BUILD", _BUILD_TEMPLATE.format(
        asciidoc = asciidoc,
        path = repository_ctx.path(""),
    ))

def symlink_command(repository_ctx, command):
    binary = repository_ctx.which(command)
    if binary == None:
        fail("Unable to find '%s' executable on path." % command)
    repository_ctx.symlink(binary, command)

external_toolchain_autoconf = repository_rule(
    implementation = _external_toolchain_autoconf_impl,
    local = True,
    environ = [
        "KYTHE_DO_NOT_DETECT_BAZEL_TOOCHAINS",
        "PATH",
    ],
)

def external_tools_configure():
    external_toolchain_autoconf(name = "local_config_tools")
    native.register_toolchains("@local_config_tools//:all")
