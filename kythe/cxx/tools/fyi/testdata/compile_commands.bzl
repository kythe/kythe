"""Rule for generating compile_commands.json.in with appropriate include directories."""

load("//tools/cpp:toolchain_utils.bzl", "find_cpp_toolchain", "use_cpp_toolchain")

_TEMPLATE = """  {{
    "directory": "OUT_DIR",
    "command": "clang++ -c {filename} -std=c++11 -Wall -Werror -I. -IBASE_DIR {system_includes}",
    "file": "{filename}",
  }}"""

def _compile_commands_impl(ctx):
    system_includes = " ".join([
        "-I{}".format(d)
        for d in find_cpp_toolchain(ctx).built_in_include_directories
    ])
    ctx.actions.write(
        output = ctx.outputs.compile_commands,
        content = "[\n{}]\n".format(",\n".join([
            _TEMPLATE.format(filename = name, system_includes = system_includes)
            for name in ctx.attr.filenames
        ])),
    )

compile_commands = rule(
    attrs = {
        "filenames": attr.string_list(
            mandatory = True,
            allow_empty = False,
        ),
        # Do not add references, temporary attribute for find_cpp_toolchain.
        # See go/skylark-api-for-cc-toolchain for more details.
        "_cc_toolchain": attr.label(
            default = Label("@bazel_tools//tools/cpp:current_cc_toolchain"),
        ),
    },
    doc = "Generates a compile_commannds.json.in template file.",
    outputs = {
        "compile_commands": "compile_commands.json.in",
    },
    toolchains = use_cpp_toolchain(),
    implementation = _compile_commands_impl,
)
