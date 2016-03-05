_FAIL_TO_FIND = """You need to have clang installed to build Kythe.
Note: Some vendors install clang with a versioned name
(like /usr/bin/clang-3.5). You can set the CLANG environment
variable to specify the full path to yours.
Please see https://kythe.io/contributing for more information.
"""

def _find_clang(ctx):
  """Find the Clang C++ compiler."""
  if "CLANG" in ctx.os.environ:
    return ctx.path(ctx.os.environ["CLANG"])
  else:
    cc = ctx.which("clang")
    if cc == None:
      fail(_FAIL_TO_FIND)
    return cc

_INC_DIR_MARKER_BEGIN = "#include <...> search starts here:"

_INC_DIR_MARKER_END = "End of search list."

def _find_cxx_include_directories(ctx, cc):
  """Compute the list of default C++ include directories."""
  result = ctx.execute([cc, "-E", "-xc++", "-", "-v"])
  start = result.stderr.find(_INC_DIR_MARKER_BEGIN)
  if start == -1:
    return []
  end = result.stderr.find(_INC_DIR_MARKER_END, start)
  if end == -1:
    return []
  inc_dirs = result.stderr[start + len(_INC_DIR_MARKER_BEGIN):end].strip()
  return [ctx.path(p.strip()) for p in inc_dirs.split("\n")]

def _get_value(it):
  """Convert `it` in serialized protobuf format."""
  if type(it) == "int":
    return str(it)
  elif type(it) == "bool":
    return "true" if it else "false"
  else:
    return "\"%s\"" % it

def _map_values(key, values):
  return "\n".join([" %s: %s" % (key, _get_value(v)) for v in values])

def _impl(ctx):
  cc = _find_clang(ctx)
  includes = _map_values(
      "cxx_builtin_include_directory",
      _find_cxx_include_directories(ctx, cc))

  ctx.symlink(Label("@//tools/cpp:BUILD"), "BUILD")
  ctx.template("clang",
               Label("@//tools/cpp:osx_gcc_wrapper.sh.in"),
               {"ADD_CXX_COMPILER": str(cc)})
  ctx.symlink("clang", "clang++"),
  ctx.template("CROSSTOOL",
               Label("@//tools/cpp:CROSSTOOL.in"),
               {"ADD_CXX_COMPILER": str(cc),
                "ADD_CXX_BUILTIN_INCLUDE_DIRECTORIES": includes,
                "ABS_WRAPPER_SCRIPT": "clang"}, False)

cc_autoconf = repository_rule(
    _impl,
    local = True,
)

def clang_configure():
  cc_autoconf(name="local_config_cc")
  native.bind(name="cc_toolchain", actual="@local_config_cc//:toolchain")
