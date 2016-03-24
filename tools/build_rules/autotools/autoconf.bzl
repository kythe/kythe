"""Skylark rules for generating files via an autotools ./configure script."""

load("@//tools/cdexec:cdexec.bzl", "rootpath")

def _fixtail(arg, prefixes):
  """If arg starts with any prefix, make tail be a root-relative path."""
  for prefix in prefixes:
    if arg.startswith(prefix):
      return arg[:len(prefix)] + rootpath(arg[len(prefix):])
  return arg

def _fixpaths(args):
  """Returns args with recognized path arguments made root-relative."""
  result = []
  munge_next = False
  for arg in args:
    if munge_next:
      munge_next = False
      result += [rootpath(arg)]
      continue
    if arg in ("-iquote", "-isystem"):
      munge_next = True
    else:
      arg = _fixtail(arg,
                     ("-I", "-L",
                      "-iquote=", "-isystem=",
                      "-iquote ", "-isystem "))
    result += [arg]
  return result

def _configure_env(cpp):
  """Returns a dict configure-relevant environment variables."""
  common = _fixpaths(cpp.compiler_options([]))
  return {
      "CC": rootpath(str(cpp.compiler_executable)),
      "CFLAGS": " ".join(common + _fixpaths(cpp.c_options)),
      "LDFLAGS": " ".join(_fixpaths(cpp.link_options)),
      "CXX": rootpath(str(cpp.compiler_executable)),
      "CXXFLAGS": " ".join(common + _fixpaths(cpp.cxx_options([]))),
  }

def _collect_env(env, deps):
  """Returns an environment dict and list of files pulled from deps.

  Args:
    env: Environment dict to update and return.
    deps: List of cc dependencies.

  Returns:
    (env, files) Where `env` is a configure environment dict updated with
    additional arguments and `files` a list of required inputs.
  """
  cflags, ldflags, depfiles = [], [], set()
  for dep in deps:
    ldflags += dep.cc.link_flags + ["-L" + lib.dirname for lib in dep.cc.libs]
    cflags += dep.cc.compile_flags
    depfiles += dep.files

  env["CFLAGS"] += " ".join([""] + _fixpaths(cflags))
  env["CXXFLAGS"] += " ".join([""] + _fixpaths(cflags))
  env["LDFLAGS"] += " ".join([""] + _fixpaths(ldflags))
  return env, list(depfiles)

def _impl(ctx):
  build_root = ctx.new_file("")
  env, deps = _collect_env(_configure_env(ctx.fragments.cpp), ctx.attr.deps)
  args = ([build_root.path, rootpath(ctx.executable.configure.path)] +
          ["%s=%s" % item for item in env.items()] +
          ctx.attr.args)

  ctx.action(
      outputs=ctx.outputs.outs + [build_root],
      inputs=(ctx.files.tools + [ctx.executable.configure] +
              ctx.files.srcs + deps),
      executable=ctx.executable._cdexec,
      progress_message="Running ./configure",
      mnemonic="GenAutotoolsConfigure",
      arguments=args,
  )

_configure_generate = rule(
    attrs = {
        "outs": attr.output_list(
            mandatory = True,
            non_empty = True,
        ),
        "deps": attr.label_list(
            providers = ["cc"],
        ),
        "srcs": attr.label_list(
            mandatory = True,
            non_empty = True,
            allow_files = True,
        ),
        "args": attr.string_list(),
        "tools": attr.label_list(
            mandatory = True,
            allow_files = True,
        ),
        "configure": attr.label(
            mandatory = True,
            executable = True,
            allow_files = True,
        ),
        "_cdexec": attr.label(
            default = Label("//tools/cdexec:cdexec"),
            executable = True,
        ),
    },
    fragments = ["cpp"],
    output_to_genfiles = True,
    implementation = _impl,
)

def configure_generate(*, name, outs, deps, srcs, args=()):
  _configure_generate(
      name=name, outs=outs, deps=deps, srcs=srcs, args=args,
      configure=":configure",
      tools=[
        ":config.guess",
        ":config.sub",
        ":install-sh",
        ":ltmain.sh",
        ":missing",
    ])
