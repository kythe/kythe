load("//kythe/docs:asciidoc.bzl", "asciidoc")

def asciidoc_with_verifier(name, src):
  """Invoke the asciidoc tool on the specified source file, filtering examples to
  be passed to the verifier. If the verifier does not succeed, the build will fail.
  """
  asciidoc(
    name = name,
    src = src,
    confs = ["kythe-filter.conf"],
    attrs = {
        # We define the filter here rather than in kythe-filter.conf, so that
        # we have access to the path of the script it wants to invoke.
        "filter": ('"$$PWD/$(location :example_sh) {backend} {style} ' +
                   '{language} {label} {graph=1} {divstyle=\'\'} ' +
                   '{verifierargs=\'\\}"'),
    },
    data = [
        ":example_sh",
        "example-clike.sh",
        "example-cxx.sh",
        "example-objc.sh",
        "example-dot.sh",
        "example-java.sh",
        "java-schema-file-data-template.FileData",
        "java-schema-unit-template.CompilationUnit",
    ],
    tags = ["manual"],
  )

def build_example_sh():
  """This rule must be executed once to set up the genrule used to plug in tool
  paths to the verifier scripts.
  """
  tools = {
      "CXX_INDEXER_BIN": "//kythe/cxx/indexer/cxx:indexer",
      "GO_INDEXER_BIN": "//kythe/go/indexer/cmd/go_example:go_example",
      "JAVA_INDEXER_BIN": "//kythe/java/com/google/devtools/kythe/analyzers/java:indexer",
      "KINDEX_TOOL_BIN": "//kythe/cxx/tools:kindex_tool",
      "VERIFIER_BIN": "//kythe/cxx/verifier",
  }
  fixes = [
      "-e '/^export %s=/{i\\\n_p=($(locations %s))\ns#$$#\"$$PWD/$${_p[0]}\"#\n}'" % (key, target)
      for (key, target) in tools.items()
  ]
  native.genrule(
      name = "example_sh",
      srcs = ["example-base.sh"] + tools.values(),
      outs = ["example.sh"],
      cmd = ' '.join(["sed"] + fixes + ["$(location example-base.sh)", ">$@"]),
  )
