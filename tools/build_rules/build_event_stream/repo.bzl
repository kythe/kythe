"""An external repository for fetching build_event_stream protobufs."""

load("@bazel_skylib//lib:paths.bzl", "paths")

_FILES = [
    "src/main/java/com/google/devtools/build/lib/buildeventstream/proto/build_event_stream.proto",
    "src/main/protobuf/command_line.proto",
    "src/main/protobuf/invocation_policy.proto",
    "src/main/protobuf/option_filters.proto",
]

_URL_TEMPLATE = "https://raw.githubusercontent.com/bazelbuild/bazel/{revision}/{path}"

_BUILD_FILE_TEMPLATE = """
load("@rules_proto//proto:defs.bzl", "proto_library")

proto_library(
    name = "build_event_stream_proto",
    srcs = [
        {filenames},
    ],
)

cc_proto_library(
    name = "build_event_stream_cc_proto",
    deps = [":build_event_stream_proto"],
    visibility = ["//visibility:public"],
)
"""

def _bes_repo(repository_ctx):
    attrs = {
        "name": repository_ctx.attr.name,
        "revision": repository_ctx.attr.revision,
        "sha256s": dict(repository_ctx.attr.sha256s),
    }
    proto_srcs = []
    for path in _FILES:
        filename = paths.basename(path)
        output = paths.join("src/main/protobuf", filename)
        proto_srcs.append("\"%s\"" % output)
        attrs["sha256s"][filename] = repository_ctx.download(
            _URL_TEMPLATE.format(
                revision = repository_ctx.attr.revision,
                path = path,
            ),
            output = output,
            sha256 = repository_ctx.attr.sha256s.get(filename, ""),
        ).sha256

    repository_ctx.file(
        "BUILD.bazel",
        _BUILD_FILE_TEMPLATE.format(
            filenames = ",\n        ".join(proto_srcs),
        ),
    )

    return attrs

build_event_stream_repository = repository_rule(
    implementation = _bes_repo,
    attrs = {
        "revision": attr.string(
            doc = "A tag, commit or branch string at which to fetch the required files.",
            default = "master",
        ),
        "sha256s": attr.string_dict(
            doc = "A mapping of basename to sha256 for the fetched files",
        ),
    },
)
