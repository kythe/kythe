load("@//tools/cdexec:cdexec.bzl", "rootpath")

def _find_cmakelists(files):
  for file in files:
    if file.basename == "CMakeLists.txt":
      return file
  return None

def _cmake_gen_impl(ctx):
  cmakefile = _find_cmakelists(ctx.files.srcs)
  if cmakefile == None:
    fail("CMakeLists.txt missing from srcs")

  cmake_cache = ctx.new_file("CMakeCache.txt")
  ctx.file_action(cmake_cache, "")

  options = ["-q", cmake_cache.dirname, "cmake"]
  if ctx.file.cache_script:
    options += ["-C", rootpath(ctx.file.cache_script.path)]
  for define in ctx.attr.defines.items():
    options += ["-D", "=".join(define)]
  options += [rootpath(cmakefile.dirname)]

  ctx.action(outputs=ctx.outputs.outs,
             inputs=ctx.files.srcs + [cmake_cache],
             mnemonic="CmakeGen",
             executable=ctx.executable._cdexec,
             use_default_shell_env=True,
             # This is necessary for cases where `cmake` or something
             # on which it depends lies outside of the sandbox.
             execution_requirements={"local": ""},
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
        "defines": attr.string_dict(),
        "cache_script": attr.label(
            single_file = True,
            allow_files = True,
        ),
        "_cdexec": attr.label(
            default = Label("//tools/cdexec:cdexec"),
            executable = True,
        ),
    },
    output_to_genfiles = True,
    implementation = _cmake_gen_impl,
)
