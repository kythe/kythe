load("@io_bazel_rules_go//go:def.bzl", _go_test="go_test")

def go_test(name, library=None, **kwargs):
  """This macro wraps the go_test rule provided by the Bazel Go rules
  to silence a deprecation warning for use of the "library" attribute.
  It is otherwise equivalent in function to a go_test.
  """
  if not library:
    fail('Missing required "library" attribute')

  _go_test(
      name = name,
      embed = [library],
      **kwargs
  )

def shell_tool_test(name, script=[], scriptfile='', tools={}, data=[],
                    args=[], size='small'):
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
  """
  if (len(script) == 0) == (scriptfile == ''):
    fail('You must set exactly one of "script" or "scriptfile"')

  bases = sorted(tools.keys())
  lines = ["set -e", 'cat >$@ <<"EOF"'] + [
      'readonly %s="$${%d:?missing %s tool}"' % (base, i+1, base)
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
      outs = [genscript+".sh"],
      srcs = [scriptfile] if scriptfile else [],
      testonly = True,
      cmd = "\n".join(lines),
  )
  test_args = ["$(location %s)" % tools[base] for base in bases]
  native.sh_test(
      name = name,
      data = sorted(tools.values()) + data,
      srcs = [genscript],
      args = test_args + args,
      size = size,
  )
