def docker_build_impl(ctx):
  args = []
  if not ctx.attr.use_cache:
    args += ['--force-rm', '--no-cache']
  cmd = '\n'.join([
      "set -e",
      "CTX=" + ctx.outputs.done_marker.path + ".ctx",
      'rm -rf "$CTX"',
      'mkdir "$CTX"',
      "srcs=(%s)" % (cmd_helper.join_paths(" ", set(ctx.files.data))),
      "for src in ${srcs[@]}; do",
      "  dir=$(dirname $src)",
      "  dir=${dir#%s}" % (ctx.configuration.bin_dir.path),
      "  dir=${dir#%s}" % (ctx.configuration.genfiles_dir.path),
      '  mkdir -p "$CTX/$dir"',
      '  cp -L --preserve=all $src "$CTX/$dir"',
      "done",
      "cp %s \"$CTX\"" % (ctx.file.src.path),
      'cd "$CTX"',
      "docker build -t %s %s ." % (ctx.attr.image_name, ' '.join(args)),
      "touch ../$(basename " + ctx.outputs.done_marker.path + ")",
    ])
  ctx.action(
      inputs = [ctx.file.src] + ctx.files.deps + ctx.files.data,
      outputs = [ctx.outputs.done_marker],
      mnemonic = 'DockerBuild',
      command = cmd,
      use_default_shell_env = True)

  return struct(dockerfile = ctx.file.src)

docker_build = rule(
    docker_build_impl,
    attrs = {
        "src": attr.label(
            allow_files = True,
            single_file = True,
        ),
        "image_name": attr.string(),
        "data": attr.label_list(allow_files = True),
        "deps": attr.label_list(
            providers = ["dockerfile"],
        ),
        "use_cache": attr.bool(),
    },
    outputs = {"done_marker": "%{name}.done"},
)
