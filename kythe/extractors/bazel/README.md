# Bazel Kythe extraction

This package contains a Docker image that configures the `gcr.io/cloud-builders/bazel` Cloud Builder
to extract Kythe `.kzip` compilations from supported targets: `gcr.io/kythe-public/bazel-extractor`

## Supported Bazel rules

* cc_{library,binary,test} (`CppCompile` action mnemonic)
* go_{library,binary,test} (`GoCompile` action mnemonic)
* java_{library,binary,test,import,proto_library} (`Javac` and `JavaIjar` action mnemonics)
* proto_library (`GenProtoDescriptorSet` action mnemonic)
* typescript_library (`TypeScriptCompile` action mnemonic)
* ng_module (`AngularTemplateCompile` action mnemonic)

## Building

```shell
bazel build -c opt --stamp //kythe/extractors/bazel:docker
```

## Example usage

```shell
git clone https://github.com/protocolbuffers/protobuf.git

mkdir -p output
docker run \
  --mount type=bind,source=$PWD/protobuf,target=/workspace/code \
  --mount type=bind,source=$PWD/output,target=/workspace/output \
  -e KYTHE_OUTPUT_DIRECTORY=/workspace/output \
  -w /workspace/code \
  gcr.io/kythe-public/bazel-extractor build \
  --define kythe_corpus=github.com/protocolbuffers/protobuf //...

kzip view output/compilations.kzip
```
