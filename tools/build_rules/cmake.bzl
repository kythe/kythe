def _find_cmakelists(files):
  for file in files:
    if file.basename == "CMakeLists.txt":
      return file
  return None

def _srcpath(path):
  return "/".join(["@_srcdir_@", path])

def _cmake_gen_impl(ctx):
  cmakefile = _find_cmakelists(ctx.files.srcs)
  if cmakefile == None:
    fail("CMakeLists.txt missing from srcs")

  relative_root = "/".join([s for s in
                   [ctx.label.workspace_root, ctx.label.package] if s])
  root = "/".join([ctx.configuration.genfiles_dir.path, relative_root])
  options = [root]
  if ctx.file.cache_script:
    options += ["-C", _srcpath(ctx.file.cache_script.path)]
  for define in ctx.attr.defines:
    options += ["-D", define]
  options += [_srcpath(cmakefile.dirname)]
  ctx.action(outputs=ctx.outputs.outs,
             inputs=ctx.files.srcs,
             mnemonic="CmakeGen",
             command=";".join([
                 "_SRCDIR=\"$(pwd)\"",
                 "cd \"$1\"",
                 "shift",
                 "touch CMakeCache.txt",
                 "cmake \"${@/#@_srcdir_@/$_SRCDIR}\" > /dev/null",
             ]),
             use_default_shell_env=True,
             arguments=options)

cmake_generate = rule(
    attrs = {
        "srcs": attr.label_list(
            allow_files = True,
            mandatory = True,
            non_empty = True,
        ),
        "outs": attr.output_list(
            mandatory = True,
            non_empty = True,
        ),
        "defines": attr.string_list(),
        "cache_script": attr.label(
            single_file = True,
            allow_files = True,
        ),
    },
    output_to_genfiles = True,
    implementation = _cmake_gen_impl,
)
