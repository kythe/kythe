# Kythe Repository Extractor Tools

## `extractrepo.go`

This tool runs a Kythe extraction on a specified repository using an extraction
configuration as defined in [kythe.proto.ExtractionConfiguration](https://github.com/kythe/kythe/blob/master/kythe/proto/extraction_config.proto).
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

### Build Instructions

To build the `extractrepo.go` binary use the following bazel command:

`bazel build //kythe/go/platform/tools/extraction:extractrepo`

It may also be necessary to build the relevant extractors that you are going to
be running:

`bazel build //kythe/java/com/google/devtools/kythe/extractors/java/artifacts`

### Usage

These instructions assume that any required Docker images referred to in the
extraction configuration file are accessible either from a local or remote
container registry from within the executing host environment.

`extractrepo -repo <repo_uri> -output <output_file_path> -config [config_file_path]`

## `repotester.go`

This tool wraps the above `extractrepo.go` tool and provides testing of input
repos, verifying that its output is sensible. It does this by separately cloning
the contents of the github repo and comparing them with the extractor's
`.kindex` output. For just extraction, it uses simple filename comparison. When
indexing is supported in the future, we will extend this tool to look at symbol
names as well.

In order to run `repotester.go`, you may first want to verify that extraction
works as above. Many of the problems you'll run into with `repotester.go` will
be exactly the same as with `extractrepo.go`, for example setting the correct
extraction config.

### Build Instructions

To build the `repotester.go` binary, use the following bazel command:

`bazel build //kythe/go/platform/tools/extraction:repotester`

### Usage

These instructions assume that any required Docker images referred to in the
extraction configuration file are accessible either from a local or remote
contanier registry from within the executing host environment.

`repotester -repos <repo_list> -config <config_file_path>`

`repo_list` can be a comma separated list of github repo urls, e.g.
`"https://github.com/kythe/kythe,https://github.com/google/go-github"`.

If you are managing a long list of repos, you can also sepecify a file with one
repo per line, and instead call:

`repotester -repos_list_file <file> ...`
