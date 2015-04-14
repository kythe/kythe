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

  gotool = ctx.file._go
  go_archive = ctx.outputs.go
  go_srcs = go_archive.path + ".go.srcs"
  protoc_gen_go = ctx.file._protoc_gen_go
  go_pkg = ctx.attr.go_package_prefix + ctx.label.package + "/" + ctx.label.name
  go_include_paths = ""
  go_deps = []
  go_recursive_deps = set()
  for dep in ctx.targets.deps + ctx.targets._proto_go_libs:
    go_include_paths += " -I \"$(dirname " + dep.go_archive.path + ")/gopath\""
    go_deps += [dep.go_archive]
    go_recursive_deps += dep.go_recursive_deps
  go_recursive_deps += go_deps

  go_symlink = ctx.new_file(ctx.configuration.bin_dir, ctx.label.name + "/gopath/" + go_pkg + ".a")
  go_symlink_content = ctx.label.name + ".a"
  for component in go_pkg.split("/"):
    go_symlink_content = "../" + go_symlink_content

  go_proto_import_path = ""
  for dep in ctx.targets.deps:
    go_proto_import_path += ",M" + dep.proto_src.path + "=kythe.io/" + dep.label.package + "/" + dep.label.name

  go_cmd = (
      "set -e;" +
      "rm -rf " + go_srcs + ";" +
      "mkdir -p " + go_srcs + ";" +
      protoc.path + " --plugin=" + protoc_gen_go.path + " --go_out=plugins=grpc,import_path=" +
      go_pkg + go_proto_import_path + ":" + go_srcs + " " + src.path + ";" +
      "find " + go_srcs + " -type f -name '*.go' -exec mv -f {} " + go_srcs + " ';';" +
      gotool.path + " tool 6g -p " + go_pkg + " -complete -pack -o " + go_archive.path + " " +
      go_include_paths + " $(ls -1 " + go_srcs + "/*.go) ;" +
      "ln -sf " + go_symlink_content + " " + go_symlink.path + ";")
  ctx.action(
      inputs = [src, protoc, protoc_gen_go, gotool] + ctx.files._proto_go_libs + go_deps,
      outputs = [go_archive, go_symlink],
      mnemonic = 'ProtocGo',
      command = go_cmd,
      use_default_shell_env = True)

  # TODO(zarko): C++ support

  return struct(proto_src = src,
                go_archive = go_archive,
                go_recursive_deps = go_recursive_deps,
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
            providers = ["proto_src"],
        ),
        # TODO(schroederc): put package prefix into common configuration file
        "go_package_prefix": attr.string(default = "kythe.io/"),
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
        "_protoc_gen_go": attr.label(
            default = Label("//third_party/go:protoc-gen-go"),
            single_file = True,
            allow_files = True,
        ),
        "_proto_go_libs": attr.label_list(
            default = [
                Label("//third_party/go:protobuf"),
                Label("//third_party/go:context"),
                Label("//third_party/go:grpc"),
            ],
            allow_files = False,
            providers = ["go_archive"],
        ),
        "_go": attr.label(
            default = Label("//tools/go"),
            allow_files = True,
            single_file = True,
        ),
    },
    outputs = {
        "java": "lib%{name}.jar",
        "go": "%{name}/%{name}.a",
    },
)
