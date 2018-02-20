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
  cmd = "bison -o $(@D)/%s $(location %s)" % (outs[0], srcs[0])
  native.genrule(
    name = name,
    outs = outs,
    srcs = srcs,
    cmd = cmd
  )
