"""Starlark library to build FlagConstructors textproto files"""

def _flag_constructors(ctx):
    flags = []
    for pkg in ctx.attr.standard_flags:
        for f in ctx.attr.standard_flags[pkg]:
            flags.append(struct(**{
                "pkg_path": pkg,
                "func_name": f,
                "name_arg_position": 0,
                "description_arg_position": 2,
            }))
    ctx.actions.write(
        output = ctx.outputs.output,
        content = proto.encode_text(struct(**{"flag": flags})),
    )

flag_constructors = rule(
    implementation = _flag_constructors,
    attrs = {
        "standard_flags": attr.string_list_dict(),
    },
    outputs = {"output": "%{name}.textproto"},
)
