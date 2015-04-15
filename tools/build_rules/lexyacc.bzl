# The 0th src of genlex must be the lexer input.
def genlex(name, srcs):
  lexer_cc = name + ".yy.cc"
  cmd = "flex -o $(@D)/%s $(location %s)" % (lexer_cc, srcs[0])
  native.genrule(
    name = name,
    outs = [lexer_cc],
    srcs = srcs,
    cmd = cmd
  )

def genyacc(name, src):
  parser_cc = name + ".yy.cc"
  parser_hh = name + ".yy.hh"
  other_hhs = ["location.hh", "stack.hh", "position.hh"]
  arg_adjust = "$$(bison --version | grep -qE '^bison .* 3\..*' && echo -Wno-deprecated)"
  cmd = "bison %s -o $(@D)/%s $(location %s)" % (arg_adjust, parser_cc, src)
  native.genrule(
    name = name,
    outs = [parser_cc, parser_hh] + other_hhs,
    srcs = [src],
    cmd = cmd
  )
