"""Repository rules for automagically configuring the host toolchain."""

_BUILD_TEMPLATE = """
package(default_visibility=["//visibility:public"])

load("@//tools/build_rules/external_tools:external_tools_toolchain.bzl", "external_tools_toolchain")

external_tools_toolchain(
  name = "host_toolchain_impl",
  asciidoc = "{asciidoc}",
  dot = "{dot}",
  python = "{python}",
  cat = "{cat}",
)

toolchain(
    name = "host_toolchain",
    exec_compatible_with = [
      # These expect constraint_setting targets, not platforms.
      # But the desired constraints are visibility restricted.
      #"@bazel_tools//platforms:host_platform",
    ],
    target_compatible_with = [
      #"@bazel_tools//platforms:host_platform",
    ],
    toolchain = ":host_toolchain_impl",
    toolchain_type = "@//tools/build_rules/external_tools:external_tools_toolchain_type",
)
"""

def _external_toolchain_autoconf_impl(repository_ctx):
    asciidoc = repository_ctx.which("asciidoc")
    if asciidoc == None:
        fail("Unable to find 'asciidoc' executable on path.")
    dot = repository_ctx.which("dot")
    if dot == None:
        fail("Unable to find 'dot' executable on path.")
    python = repository_ctx.which("python")
    if python == None:
        fail("Unable to find 'python' executable on path.")
    cat = repository_ctx.which("cat")
    if cat == None:
        fail("Unable to find 'cat' executable on path.")
    repository_ctx.file("BUILD", _BUILD_TEMPLATE.format(asciidoc = asciidoc, dot = dot, python = python, cat = cat))

external_toolchain_autoconf = repository_rule(
    implementation = _external_toolchain_autoconf_impl,
    local = True,
    environ = ["PATH"],
)

def external_tools_configure():
    external_toolchain_autoconf(name = "local_config_tools")
    native.register_toolchains("@local_config_tools//:host_toolchain")
