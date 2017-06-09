def expand_template_impl(ctx):
  ctx.template_action(
      template = ctx.file.template,
      output = ctx.outputs.out,
      substitutions = ctx.attr.substitutions,
      executable = False,
  )

expand_template = rule(
    attrs = {
        "template": attr.label(
            mandatory = True,
            allow_files = True,
            single_file = True,
        ),
        "substitutions": attr.string_dict(mandatory = True),
        "out": attr.output(mandatory = True),
    },
    output_to_genfiles = True,
    implementation = expand_template_impl,
)
