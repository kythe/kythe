# The 0th src of genlex must be the lexer input.
def genlex(name, srcs, out):
    cmd = "flex -o $(@D)/%s $(location %s)" % (out, srcs[0])
    native.genrule(
        name = name,
        outs = [out],
        srcs = srcs,
        cmd = cmd,
    )

def genyacc(name, src, header_out, source_out, extra_outs = []):
    """Generate a C++ parser from a Yacc file using Bison.

    Args:
      name: The name of the rule.
      src: The input grammar file.
      header_out: The generated header file.
      source_out: The generated source file.
      extra_outs: Additional generated outputs.
    """
    cmd = "bison -o $(@D)/%s $(location %s)" % (source_out, src)
    native.genrule(
        name = name,
        outs = [source_out, header_out] + extra_outs,
        srcs = [src],
        cmd = cmd,
    )
