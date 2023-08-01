load("@bazel_skylib//lib:versions.bzl", "versions")

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
        toolchains = ["@io_kythe//tools/build_rules/lexyacc:current_lexyacc_toolchain"],
        tools = ["@io_kythe//tools/build_rules/lexyacc:current_lexyacc_toolchain"],
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
        toolchains = ["@io_kythe//tools/build_rules/lexyacc:current_lexyacc_toolchain"],
        tools = ["@io_kythe//tools/build_rules/lexyacc:current_lexyacc_toolchain"],
    )

LexYaccInfo = provider(
    doc = "Paths to lex and yacc binaries.",
    fields = ["lex", "yacc", "runfiles"],
)

def _lexyacc_variables(ctx):
    lyinfo = ctx.toolchains["@io_kythe//tools/build_rules/lexyacc:toolchain_type"].lexyaccinfo
    return [
        DefaultInfo(runfiles = lyinfo.runfiles, files = lyinfo.runfiles.files),
        platform_common.TemplateVariableInfo({
            "LEX": lyinfo.lex,
            "YACC": lyinfo.yacc,
        }),
    ]

lexyacc_variables = rule(
    implementation = _lexyacc_variables,
    toolchains = ["@io_kythe//tools/build_rules/lexyacc:toolchain_type"],
)

def _lexyacc_toolchain_impl(ctx):
    runfiles = ctx.runfiles()
    if bool(ctx.attr.lex) == bool(ctx.attr.lex_path):
        fail("Exactly one of lex and lex_path is required")

    if ctx.attr.lex:
        runfiles = runfiles.merge(ctx.attr.lex[DefaultInfo].default_runfiles)

    if bool(ctx.attr.yacc) == bool(ctx.attr.yacc_path):
        fail("Exactly one of yacc and yacc_path is required")

    if ctx.attr.yacc:
        runfiles = runfiles.merge(ctx.attr.yacc[DefaultInfo].default_runfiles)

    return [
        platform_common.ToolchainInfo(
            lexyaccinfo = LexYaccInfo(
                lex = ctx.attr.lex_path or ctx.executable.lex.path,
                yacc = ctx.attr.yacc_path or ctx.executable.yacc.path,
                runfiles = runfiles,
            ),
        ),
    ]

_lexyacc_toolchain = rule(
    implementation = _lexyacc_toolchain_impl,
    attrs = {
        "lex_path": attr.string(),
        "yacc_path": attr.string(),
        "lex": attr.label(
            executable = True,
            cfg = "exec",
        ),
        "yacc": attr.label(
            executable = True,
            cfg = "exec",
        ),
    },
    provides = [
        platform_common.ToolchainInfo,
    ],
)

def lexyacc_toolchain(name, **kwargs):
    _lexyacc_toolchain(name = name, **kwargs)
    native.toolchain(
        name = name + "_toolchain",
        toolchain = ":" + name,
        toolchain_type = "@io_kythe//tools/build_rules/lexyacc:toolchain_type",
    )

def _check_flex_version(repository_ctx, min_version):
    flex = repository_ctx.os.environ.get("FLEX", repository_ctx.which("flex"))
    if flex == None:
        fail("Unable to find flex binary")
    flex_result = repository_ctx.execute([flex, "--version"])
    if flex_result.return_code:
        fail("Unable to determine flex version: " + flex_result.stderr)
    flex_version = flex_result.stdout.split(" ")
    if len(flex_version) < 2:
        fail("Too few components in flex version: " + flex_result.stdout)
    if not versions.is_at_least(min_version, flex_version[1]):
        fail("Flex too old (%s < %s)" % (flex_version[1], min_version))
    return flex

def _local_lexyacc(repository_ctx):
    if repository_ctx.os.environ.get("KYTHE_DO_NOT_DETECT_BAZEL_TOOLCHAINS", "0") == "1":
        repository_ctx.file("BUILD.bazel", "# Toolchain detection disabled by KYTHE_DO_NOT_DETECT_BAZEL_TOOLCHAINS")
        return
    flex = _check_flex_version(repository_ctx, "2.6")
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
            "load(\"@io_kythe//tools/build_rules/lexyacc:lexyacc.bzl\", \"lexyacc_toolchain\")",
            "package(default_visibility=[\"//visibility:public\"])",
            "lexyacc_toolchain(",
            "  name = \"lexyacc_local\",",
            "  lex_path = \"%s\"," % flex,
            "  yacc_path = \"%s\"," % bison,
            ")",
        ]),
    )

local_lexyacc_repository = repository_rule(
    implementation = _local_lexyacc,
    local = True,
    environ = [
        "KYTHE_DO_NOT_DETECT_BAZEL_TOOCHAINS",
        "PATH",
        "BISON",
        "FLEX",
    ],
)

def lexyacc_configure():
    local_lexyacc_repository(name = "local_config_lexyacc")
    native.register_toolchains("@local_config_lexyacc//:all")
