# Kythe Repository Extractor

This tool runs a Kythe extraction on a specified repository using an extraction
configuration as defined in [kythe.proto.ExtractionConfiguration](https://github.com/google/kythe/blob/ecbaa951f2cee00ad9f6e3165b078badf02dd38b/kythe/proto/extraction_config.proto).
It makes a local clone of the repository via the specified repository URI, and 
runs an extraction on its contents. It will accept an extraction configuration
file specified as a CLI argument, otherwise it will search the root of repository
for a Kythe configuration file named: ".kythe-extraction-config". The extraction
configuration is expected to be in the format of [JSON encoded protobuf](https://developers.google.com/protocol-buffers/docs/proto3#json).
The extraction requirements specified within the configuration will be drawn
upon to compose a customized extraction Docker image containing the necessary
build systems and SDKs for compiling and extracting Kythe compilation units for
the project. The tool proceeds to build the customized extraction image and
execute it for performing the extraction process. The output of the extraction
process are [kindex files](http://kythe.io/docs/kythe-index-pack.html).

## Build Instructions

To build the extractrepo.go binary use the following bazel command:
`bazel build //kythe/go/platform/tools/extraction:extractrepo`

## Usage

These instructions assume that any required Docker images referred to in the
extraction configuration file are accessible either from a local or remote
container registry from within the executing host environment.

`extractrepo -repo <repo_uri> -output <output_file_path> -config [config_file_path]`
