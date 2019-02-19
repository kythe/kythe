def genlex(name, src, out, includes = []):
    """Generate a C++ lexer from a lex file using Flex.

    Args:
      name: The name of the rule.
      src: The .lex source file.
      out: The generated source file.
      includes: A list of headers included by the .lex file.
    """
    cmd = "flex -o $(@D)/%s $(location %s)" % (out, src)
    native.genrule(
        name = name,
        outs = [out],
        srcs = [src] + includes,
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
    cmd = "$${BISON:-bison} -o $(@D)/%s $(location %s)" % (source_out, src)
    native.genrule(
        name = name,
        outs = [source_out, header_out] + extra_outs,
        srcs = [src],
        cmd = cmd,
    )
