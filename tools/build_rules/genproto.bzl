jar_filetype = FileType([".jar"])

def genproto_impl(ctx):
  src = ctx.file.src
  protoc = ctx.file._protoc

  compile_time_jars = set([ctx.file._protoc_gen_java], order="link")
  runtime_jars = set([ctx.file._protoc_gen_java], order="link")
  for dep in ctx.targets.deps:
    compile_time_jars += dep.compile_time_jars
    runtime_jars += dep.runtime_jars
  jdk_bin = "tools/jdk/jdk/bin/"
  jar_out = ctx.outputs.java
  java_srcs = jar_out.path + ".srcs"
  java_classes = jar_out.path + ".classes"

  java_cmd = (
         "set -e;" +
         "rm -rf " + java_srcs + " " + java_classes + ";" +
         "mkdir " + java_srcs + " " + java_classes +  ";" +
         protoc.path + " --java_out=" + java_srcs + " " + src.path + ";" +
         jdk_bin + "javac -encoding utf-8 -cp '" + cmd_helper.join_paths(":", compile_time_jars) +
         "' -d " + java_classes + " $(find " + java_srcs + " -name '*.java');" +
         jdk_bin + "jar cf " + jar_out.path + " -C " + java_classes + " .;")
  ctx.action(
      inputs = [src, protoc] + list(compile_time_jars),
      outputs = [jar_out],
      mnemonic = 'ProtocJava',
      command = java_cmd,
      use_default_shell_env = True)

  # TODO(schroederc): C++ support
  # TODO(schroederc): Go support

  return struct(proto_defs = set([src]),
                compile_time_jars = set([jar_out]) + compile_time_jars,
                runtime_jars = set([jar_out], order="link") + runtime_jars)

# genproto name is required for interop w/ native Bazel rules
genproto = rule(
    genproto_impl,
    attrs = {
        "src": attr.label(
            allow_files = FileType([".proto"]),
            single_file = True,
        ),
        "deps": attr.label_list(
            allow_files = False,
            providers = [
                "proto_defs",
                "compile_time_jars",
                "runtime_jars",
            ],
        ),
        "_protoc": attr.label(
            default = Label("//third_party:protoc"),
            allow_files = True,
            single_file = True,
        ),
        "_protoc_gen_java": attr.label(
            default = Label("//third_party:protobuf"),
            single_file = True,
            allow_files = jar_filetype,
        ),
    },
    outputs = {"java": "lib%{name}.jar"},
)
