def genlex(name, src, out, includes = []):
    """Generate a C++ lexer from a lex file using Flex.

    Args:
      name: The name of the rule.
      src: The .lex source file.
      out: The generated source file.
      includes: A list of headers included by the .lex file.
    """
    cmd = "$(LEX) -o $(@D)/%s $(location %s)" % (out, src)
    native.genrule(
        name = name,
        outs = [out],
        srcs = [src] + includes,
        cmd = cmd,
        toolchains = ["@//tools/build_rules/lexyacc:current_lexyacc_toolchain"],
    )

def genyacc(name, src, header_out, source_out, extra_outs = []):
    """Generate a C++ parser from a Yacc file using Bison.

    Args:
      name: The name of the rule.
      src: The input grammar file.
      header_out: The generated header file.
      source_out: The generated source file.
      extra_outs: Additional generated outputs.
    """
    cmd = "$(YACC) -o $(@D)/%s $(location %s)" % (source_out, src)
    native.genrule(
        name = name,
        outs = [source_out, header_out] + extra_outs,
        srcs = [src],
        cmd = cmd,
        toolchains = ["@//tools/build_rules/lexyacc:current_lexyacc_toolchain"],
    )

LexYaccInfo = provider(
    doc = "Paths to lex and yacc binaries.",
    fields = ["lex", "yacc"],
)

def _lexyacc_variables(ctx):
    lyinfo = ctx.toolchains["//tools/build_rules/lexyacc:toolchain_type"].lexyaccinfo
    return [
        platform_common.TemplateVariableInfo({
            "LEX": lyinfo.lex,
            "YACC": lyinfo.yacc,
        }),
    ]

lexyacc_variables = rule(
    implementation = _lexyacc_variables,
    toolchains = ["//tools/build_rules/lexyacc:toolchain_type"],
)

def _lexyacc_toolchain(ctx):
    return [
        platform_common.ToolchainInfo(
            lexyaccinfo = LexYaccInfo(
                lex = ctx.attr.lex,
                yacc = ctx.attr.yacc,
            ),
        ),
    ]

lexyacc_toolchain = rule(
    implementation = _lexyacc_toolchain,
    attrs = {
        "lex": attr.string(),
        "yacc": attr.string(),
    },
    provides = [
        platform_common.ToolchainInfo,
    ],
)

def _local_lexyacc(repository_ctx):
    flex = repository_ctx.which("flex")
    if flex == None:
        fail("Unable to find flex binary")
    bison = repository_ctx.os.environ.get("BISON", repository_ctx.which("bison"))
    if not bison:
        fail("Unable to find bison binary")
    repository_ctx.file(
        "WORKSPACE",
        content = "workspace(name=\"%s\")" % (repository_ctx.name,),
        executable = False,
    )
    repository_ctx.file(
        "BUILD.bazel",
        content = "\n".join([
            "load(\"@//tools/build_rules/lexyacc:lexyacc.bzl\", \"lexyacc_toolchain\")",
            "package(default_visibility=[\"//visibility:public\"])",
            "lexyacc_toolchain(",
            "  name = \"lexyacc_local\",",
            "  lex = \"%s\"," % flex,
            "  yacc = \"%s\"," % bison,
            ")",
            "toolchain(",
            "  name = \"lexyacc_local_toolchain\",",
            "  toolchain = \":lexyacc_local\",",
            "  toolchain_type = \"@//tools/build_rules/lexyacc:toolchain_type\",",
            "  #exec_compatible_with = [\"@bazel_tools//platforms:host_platform\"],",
            "  #target_compatible_with = [\"@bazel_tools//platforms:target_platform\"],",
            ")",
        ]),
    )

local_lexyacc_repository = repository_rule(
    implementation = _local_lexyacc,
    local = True,
    environ = ["PATH", "BISON"],
)

def lexyacc_configure():
    local_lexyacc_repository(name = "local_config_lexyacc")
    native.register_toolchains(
        "@local_config_lexyacc//:lexyacc_local_toolchain",
    )
