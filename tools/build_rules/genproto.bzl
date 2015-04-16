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

  java_grpc_cmd = ""
  if ctx.attr.has_services:
    java_grpc_plugin = ctx.file._protoc_grpc_plugin_java
    compile_time_jars += ctx.files._proto_grpc_java_libs
    runtime_jars += ctx.files._proto_grpc_java_libs
    java_grpc_cmd = (
        protoc.path + " --java_rpc_out=" + java_srcs +
        " --plugin=protoc-gen-java_rpc=" + java_grpc_plugin.path + " " + src.path + "\n")

  java_cmd = (
      "set -e;" +
      "rm -rf " + java_srcs + " " + java_classes + ";" +
      "mkdir " + java_srcs + " " + java_classes +  "\n" +
      protoc.path + " --java_out=" + java_srcs + " " + src.path + "\n" +
      java_grpc_cmd +
      jdk_bin + "javac -encoding utf-8 -cp '" + cmd_helper.join_paths(":", compile_time_jars) +
      "' -d " + java_classes + " $(find " + java_srcs + " -name '*.java')\n" +
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

  go_libs = ctx.targets.deps + ctx.targets._proto_go_libs
  if ctx.attr.has_services:
    go_libs += ctx.targets._proto_grpc_go_libs

  go_include_paths = ""
  go_deps = []
  go_recursive_deps = set()
  for dep in go_libs:
    go_include_paths += " -I \"" + dep.go_archive.path + "_gopath\""
    go_deps += [dep.go_archive]
    go_recursive_deps += dep.go_recursive_deps
  go_recursive_deps += go_deps

  go_symlink = ctx.new_file(ctx.configuration.bin_dir, ctx.label.name + ".a_gopath/" + go_pkg + ".a")
  go_symlink_content = ctx.label.name + ".a"
  for component in go_pkg.split("/"):
    go_symlink_content = "../" + go_symlink_content

  go_proto_import_path = ""
  for dep in ctx.targets.deps:
    go_proto_import_path += ",M" + dep.proto_src.path + "=" + ctx.attr.go_package_prefix + dep.label.package + "/" + dep.label.name

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
      inputs = [src, protoc, protoc_gen_go, gotool] + go_deps,
      outputs = [go_archive, go_symlink],
      mnemonic = 'ProtocGo',
      command = go_cmd,
      use_default_shell_env = True)

  return struct(proto_src = src,
                go_archive = go_archive,
                go_recursive_deps = go_recursive_deps,
                compile_time_jars = set([jar_out]) + compile_time_jars,
                runtime_jars = set([jar_out], order="link") + runtime_jars,
                # Provide "files" as a hack to get proper runtime_deps for java_binary rules
                files = set([jar_out, go_archive]) + compile_time_jars + go_recursive_deps)

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
        "has_services": attr.int(),
        # TODO(schroederc): put package prefix into common configuration file
        "go_package_prefix": attr.string(default = "kythe.io/"),
        "_protoc": attr.label(
            default = Label("//third_party/proto:protoc"),
            allow_files = True,
            single_file = True,
        ),
        "_protoc_gen_java": attr.label(
            default = Label("//third_party/proto:protobuf_java"),
            single_file = True,
            allow_files = jar_filetype,
        ),
        "_protoc_grpc_plugin_java": attr.label(
            default = Label("//third_party/grpc-java:plugin"),
            single_file = True,
        ),
        "_protoc_gen_go": attr.label(
            default = Label("//third_party/go:protoc-gen-go"),
            single_file = True,
            allow_files = True,
        ),
        "_proto_go_libs": attr.label_list(
            default = [Label("//third_party/go:protobuf")],
            allow_files = False,
            providers = ["go_archive"],
        ),
        "_proto_grpc_go_libs": attr.label_list(
            default = [
                Label("//third_party/go:context"),
                Label("//third_party/go:grpc"),
            ],
            allow_files = False,
            providers = ["go_archive"],
        ),
        "_proto_grpc_java_libs": attr.label_list(
            default = [
                Label("//third_party/grpc-java"),
                Label("//third_party/guava"),
                Label("//third_party/jsr305_annotations:jsr305"),
            ],
            allow_files = jar_filetype,
        ),
        "_go": attr.label(
            default = Label("//tools/go"),
            allow_files = True,
            single_file = True,
        ),
    },
    outputs = {
        "java": "lib%{name}.jar",
        "go": "%{name}.a",
    },
)

def genproto_all(name, src, deps = [], has_services = None):
  genproto(name = name, src = src, deps = deps, has_services = has_services)
  # We'll guess that the repository is set up such that a .proto in
  # //foo/bar has the package foo.bar. `location` is substituted with the
  # relative path to its label from the workspace root.
  if len(name) < 6 or name[-6:] != "_proto":
    fail("genproto names should end in _proto, saw %s" % name, "name")
  proto_path = "$(location %s)" % src
  proto_hdr = src[0:-6] + ".pb.h"
  proto_src = src[0:-6] + ".pb.cc"
  proto_srcgen_rule = name + "_cc_protoc"
  proto_lib = name + "_cc"
  protoc = "//third_party/proto:protoc"
  proto_cmd = "$(location %s) --cpp_out=$(GENDIR)/ %s" % (protoc, src)
  cc_deps = ["//third_party/proto:protobuf"]
  proto_deps = [src, protoc]
  for dep in deps:
    if len(dep) > 6 and dep[-6:] == "_proto":
      cc_deps += [dep + "_cc"]
      proto_deps += [dep]
    else:
      cc_deps += [dep]
  native.genrule(
    name = proto_srcgen_rule,
    outs = [proto_hdr, proto_src],
    srcs = proto_deps,
    cmd = proto_cmd
  )
  native.cc_library(
    name = proto_lib,
    hdrs = [proto_hdr],
    srcs = [":" + proto_srcgen_rule],
    defines = ["GOOGLE_PROTOBUF_NO_RTTI"],
    deps = cc_deps
  )
