"""Starlark library to build FlagConstructors textproto files"""

def flag_constructor(func_name, pkg_path, name_arg_position, description_arg_position):
    """Produces a text proto encoding of a kythe.proto.FlagConstructor.  Meant
    to be used as a custom_flags value.

    Args:
      func_name: the name of the flag constructing function
      pkg_path: the package path of the flag constructing function
      name_arg_position: the 0-based position of the name string argument
      description_arg_position: the 0-based position of the description string argument
    """
    return proto.encode_text(struct(
        func_name = func_name,
        pkg_path = pkg_path,
        name_arg_position = name_arg_position,
        description_arg_position = description_arg_position,
    ))

def _flag_constructors(ctx):
    textpb = "# proto-file: kythe/proto/go.proto\n# proto-message: kythe.proto.FlagConstructors\n\n"
    flags = []
    for pkg in ctx.attr.standard_flags:
        for f in ctx.attr.standard_flags[pkg]:
            flags.append(struct(**{
                "pkg_path": pkg,
                "func_name": f,
                "name_arg_position": 0,
                "description_arg_position": 2,
            }))
    for pkg in ctx.attr.standard_var_flags:
        for f in ctx.attr.standard_var_flags[pkg]:
            flags.append(struct(**{
                "pkg_path": pkg,
                "func_name": f,
                "name_arg_position": 1,
                "description_arg_position": 3,
            }))
    textpb += proto.encode_text(struct(**{"flag": flags}))
    for f in ctx.attr.custom_flags:
        textpb += "flag {\n  " + f.replace("\n", "\n  ").strip() + "\n}\n"
    ctx.actions.write(
        output = ctx.outputs.output,
        content = textpb,
    )

flag_constructors = rule(
    implementation = _flag_constructors,
    attrs = {
        "standard_flags": attr.string_list_dict(),
        "standard_var_flags": attr.string_list_dict(),
        "custom_flags": attr.string_list(),
    },
    outputs = {"output": "%{name}.textproto"},
)
