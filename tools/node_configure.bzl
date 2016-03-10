_BUILD_CONTENT = """
exports_files(["node"])
"""

def _find_node(ctx):
  if "NODEJS" in ctx.os.environ:
    return ctx.path(ctx.os.environ["NODEJS"])
  node = ctx.which("node")
  if node == None:
    print("No node.js installation found.")
    return ctx.which("false")
  else:
    return node

def _impl(ctx):
  node = _find_node(ctx)
  ctx.symlink(node, "node")
  ctx.file("BUILD", _BUILD_CONTENT, False)

node_autoconf = repository_rule(
    _impl,
    local = True,
)

def node_configure():
  node_autoconf(name="local_node")
  native.bind(name="node", actual="@local_node//:node")
