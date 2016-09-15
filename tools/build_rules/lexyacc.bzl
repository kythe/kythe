# The 0th src of genlex must be the lexer input.
def genlex(name, srcs, out):
  cmd = "flex -o $(@D)/%s $(location %s)" % (out, srcs[0])
  native.genrule(
    name = name,
    outs = [out],
    srcs = srcs,
    cmd = cmd
  )

def genyacc(name, srcs, outs):
  arg_adjust = "$$(bison --version | grep -qE '^bison .* 3\..*' && echo -Wno-deprecated)"
  cmd = "bison %s -o $(@D)/%s $(location %s)" % (arg_adjust, outs[0], srcs[0])
  native.genrule(
    name = name,
    outs = outs,
    srcs = srcs,
    cmd = cmd
  )
