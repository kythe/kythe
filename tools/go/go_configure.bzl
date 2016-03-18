_NO_GO = """You need to have go installed to build Kythe.
Please see https://kythe.io/contributing for more information.
"""

def _find_goroot(ctx):
  if "GOROOT" in ctx.os.environ:
    return ctx.path(ctx.os.environ["GOROOT"])
  go = ctx.which("go")
  if go == None:
    fail(_NO_GO)
  result = ctx.execute([go, "env", "GOROOT"])
  return ctx.path(result.stdout.strip())

def _symlink_contents(ctx, goroot):
  for child in goroot.readdir():
    ctx.symlink(child, child.basename)

def _impl(ctx):
  _symlink_contents(ctx, _find_goroot(ctx))
  ctx.symlink(Label("@//tools/go:goroot.BUILD"), "BUILD")

go_autoconf = repository_rule(
    _impl,
    local = True,
)

def go_configure():
  # TODO(shahms): Change this to local_config_go.
  # However, go.bzl references this path directly rather than using
  # the location of the //external:default-goroot or equivalent.
  go_autoconf(name="local_goroot")
  native.bind(name="goroot", actual="@local_goroot//:default-goroot")
  native.bind(name="gotool", actual="@local_goroot//:gotool")
