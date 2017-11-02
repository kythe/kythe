"""Rule for generating compile_commands.json.in with appropriate inlcude directories."""

_TEMPLATE = """  {{
    "directory": "OUT_DIR",
    "command": "clang++ -c {filename} -std=c++11 -Wall -Werror -I. -IBASE_DIR {system_includes}",
    "file": "{filename}",
  }}"""

def _compile_commands_impl(ctx):
  system_includes = " ".join([
      "-I{}".format(d)
      for d in ctx.fragments.cpp.built_in_include_directories
  ])
  ctx.actions.write(
      output = ctx.outputs.compile_commands,
      content = "[\n{}]\n".format(",\n".join([
          _TEMPLATE.format(filename=name, system_includes=system_includes)
          for name in ctx.attr.filenames
      ])),
  )

compile_commands = rule(
    attrs = {
        "filenames": attr.string_list(
            mandatory = True,
            allow_empty = False,
        ),
    },
    doc = "Generates a compile_commannds.json.in template file.",
    fragments = ["cpp"],
    outputs = {
        "compile_commands": "compile_commands.json.in",
    },
    implementation = _compile_commands_impl,
)
