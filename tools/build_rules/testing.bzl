"""This module defines macros for making shell tests."""

def file_diff_test(name, file1, file2, message = "", size = "small", tags = []):
    """A macro to compare a source file with a generated counterpart.

    This macro generates a sh_test that prints a diff and fails if file1 is not
    the same as file2.

    Args:
      name: the name of the sh_test to create
      file1: the left-hand file to compare
      file2: the right-hand file to compare
      message: optional message to print if the files differ
      size: the size of the test
      tags: tags for the sh_test rule
    """
    if file1 == "" or file2 == "":
        fail('You must provide both "file1" and "file2"')

    if message == "":
        message = "Files $(location %s) and $(location %s) differ" % (file1, file2)

    gen = name + "_script"
    native.genrule(
        name = gen,
        srcs = [file1, file2],
        outs = [gen + ".sh"],
        executable = True,
        testonly = True,
        visibility = ["//visibility:private"],
        cmd = "\n".join([
            "if ! diff '$(location %s)' '$(location %s)' ; then" % (file1, file2),
            "  echo 'printf \"\\n>> %s\\n\\n\" ; exit 1' > $@" % message,
            "else",
            "  echo 'exit 0' > $@",
            "fi",
        ]),
    )
    native.sh_test(
        name = name,
        srcs = [":" + gen],
        size = size,
        tags = tags,
    )

def shell_tool_test(
        name,
        script = [],
        scriptfile = "",
        tools = {},
        data = [],
        args = [],
        size = "small",
        tags = []):
    """A macro to invoke a test script with access to specified tools.

    Each tool named by the tools attribute is available in the script via a
    variable named by the corresponding key. For example, if tools contains

       "A": "//foo/bar:baz"

    then $A refers to the location of //foo/bar:baz in the script. Each element
    of script becomes a single line of the resulting test.

    Exactly one of "script" or "scriptfile" must be set. If "script" is set, it
    is used as an inline script; otherwise "scriptfile" must be a file target
    containing the script to be included.

    Any targets specified in "data" are included as data dependencies for the
    resulting sh_test rule without further interpretation.

    Args:
      name: the name for the sh_test to create
      script: the script to use.
        Either script or scriptfile must be set, but not both.
      scriptfile: the file containing the script to use.
        Either scriptfile or script must be set, but not both.
      tools: tools to use, passed in to test_args and data
      data: additional data to pass in to the sh_test
      args: additinoal args to pass in to the sh_test
      size: size of the test ("small", "medium", etc)
      tags: tags for the sh_test
    """
    if (len(script) == 0) == (scriptfile == ""):
        fail('You must set exactly one of "script" or "scriptfile"')

    bases = sorted(tools.keys())
    lines = ["set -e", 'cat >$@ <<"EOF"'] + [
        'readonly %s="$${%d:?missing %s tool}"' % (base, i + 1, base)
        for i, base in enumerate(bases)
    ] + ["shift %d" % len(bases)] + script + ["EOF"]
    if scriptfile:
        lines += [
            "# --- end generated section ---",
            'cat >>$@ "$(location %s)"' % scriptfile,
        ]

    genscript = name + "_test_script"
    native.genrule(
        name = genscript,
        outs = [genscript + ".sh"],
        srcs = [scriptfile] if scriptfile else [],
        testonly = True,
        cmd = "\n".join(lines),
        tags = tags,
    )
    test_args = ["$(location %s)" % tools[base] for base in bases]
    native.sh_test(
        name = name,
        data = sorted(tools.values()) + data,
        srcs = [genscript],
        args = test_args + args,
        size = size,
        tags = tags,
    )
