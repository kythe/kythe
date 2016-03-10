_BUILD_TEMPLATE = """
cc_library(
  name = "{target}",
  srcs = ["empty.cc"],
  hdrs = glob(["include/**"]),
  includes = {includes},
  linkopts = {linkopts},
  visibility = ["//visibility:public"],
)
"""

_FAIL_TEMPLATE = """Could not find {target}.
Set the {env} variable and try again.
"""

def _local_dir_path(ctx, path):
  # TODO(shahms): use $(location) on resolution of
  # https://github.com/bazelbuild/bazel/issues/1023
  return "external/local_%s/%s" % (ctx.attr.target, path)

def _remove_prefix(value, prefix):
  if value.startswith(prefix):
    return value[len(prefix):]
  else:
    return value

def _fail_to_find(ctx):
  fail(_FAIL_TEMPLATE.format(target=ctx.attr.modname,
                             env=ctx.attr.darwin_root_var))

def _find_home(ctx):
  if ctx.attr.darwin_root_var in ctx.os.environ:
    return ctx.path(ctx.os.environ[ctx.attr.darwin_root_var])
  return ctx.path(ctx.attr.darwin_root_default)

def _is_darwin(ctx):
  if ctx.os.name.lower().startswith("mac os"):
    return True
  else:
    return False

def _pkg_config_execute(ctx, pkg_config, args):
  result = ctx.execute([pkg_config] + args)
  if result.return_code != 0:
    fail(result.stderr)
  return result.stdout

def _pkg_config_includes(ctx, pkg_config, modname):
  stdout = _pkg_config_execute(ctx, pkg_config, ["--cflags-only-I", modname])
  return [_remove_prefix(i, "-I") for i in stdout.strip().split(" ") if i]

def _pkg_config_linkopts(ctx, pkg_config, modname):
  stdout = _pkg_config_execute(ctx, pkg_config, ["--libs", modname])
  return [arg for arg in stdout.strip().split(" ") if arg]

def _link_include_dirs(ctx, includes):
  return _link_directories(ctx, includes, "include")

def _link_library_dirs(ctx, libraries):
  return _link_directories(ctx, libraries, "lib", prefix="-L")

def _link_directories(ctx, arguments, subdir, prefix=""):
  result, directories = [], []
  for arg in arguments:
    if arg.startswith(prefix) and _remove_prefix(arg, prefix).startswith("/"):
      inpath, outpath = ctx.path(_remove_prefix(arg, prefix)), ctx.path(subdir)
      ctx.symlink(inpath, outpath.get_child(inpath.basename))
      directories += ["%s/%s" % (subdir, inpath.basename)]
      result += [_local_dir_path(ctx, "%s/%s" % (subdir, inpath.basename))]
    else:
      result += [arg]
  return result, directories

def _config_lib_darwin(ctx):
  # TODO(shahms): Allow users to override this for more than just OS X.
  home = _find_home(ctx)
  # We mandate a {ROOT}/include and {ROOT}/lib structure.
  if not home.get_child("include").exists:
    _fail_to_find(ctx)
  directories = ["lib", "include"]
  for dirname in directories:
    ctx.symlink(home.get_child(dirname), dirname)
  return {
      "target": ctx.attr.target,
      "includes": repr(["include"]),
      "linkopts": repr(["-L" + _local_dir_path(ctx, "lib"),
                        "-l" + _remove_prefix(ctx.attr.target, "lib")]),
  }

def _config_lib(ctx):
  pkg_config = ctx.which("pkg-config")
  if pkg_config == None:
    fail("Unable to find pkg-config binary.")
  _link_include_dirs(
      ctx, _pkg_config_includes(ctx, pkg_config, ctx.attr.modname))

  linkopts, _ = _link_library_dirs(
      ctx, _pkg_config_linkopts(ctx, pkg_config, ctx.attr.modname))

  return {
      "target": ctx.attr.target,
      # We're uisng incdirs here as includes expects a list
      # of paths relative to the BUILD file, rather than exec root
      # as would the other attrs and none of them support $(location)
      "includes": "glob([\"include/*\"], exclude_directories=0)",
      "linkopts": repr(linkopts),
  }

def _impl(ctx):
  if _is_darwin(ctx):
    content = _config_lib_darwin(ctx)
  else:
    content = _config_lib(ctx)
  ctx.file("BUILD", _BUILD_TEMPLATE.format(**content), False)
  # TODO(shahms): See if we can use the library directly on OS X.
  ctx.file("empty.cc", "", False)

kythe_pkg_config_repository = repository_rule(
    _impl,
    attrs = {
        "modname": attr.string(mandatory = True),
        "target": attr.string(),
        "darwin_root_var": attr.string(),
        "darwin_root_default": attr.string(),
    },
    local = True,
)

def kythe_pkg_config(name, darwin, modname=None):
  if modname == None:
    modname = name
  root_var, root_default = darwin
  kythe_pkg_config_repository(name="local_{name}".format(name=name),
                              modname=modname,
                              target=name,
                              darwin_root_var=root_var,
                              darwin_root_default=root_default)
  native.bind(name=name, actual="@local_{name}//:{name}".format(name=name))
