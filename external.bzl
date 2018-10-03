load("@bazel_tools//tools/build_defs/repo:git.bzl", "git_repository")
load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")
load("@bazel_tools//tools/build_defs/repo:maven_rules.bzl", "maven_jar")

def maybe(repo_rule, name, **kwargs):
    """Defines a repository if it does not already exist.
    """
    if name not in native.existing_rules():
        repo_rule(name = name, **kwargs)

def _cc_dependencies():
    maybe(
        http_archive,
        name = "net_zlib",
        build_file = "@io_kythe//third_party:zlib.BUILD",
        sha256 = "c3e5e9fdd5004dcb542feda5ee4f0ff0744628baf8ed2dd5d66f8ca1197cb1a1",
        strip_prefix = "zlib-1.2.11",
        urls = ["https://zlib.net/zlib-1.2.11.tar.gz"],
    )

def _java_dependencies():
    maybe(
        # For @com_google_common_flogger
        http_archive,
        name = "google_bazel_common",
        strip_prefix = "bazel-common-e7580d1db7466e6c8403f7826b7558ea5e99bbfd",
        urls = ["https://github.com/google/bazel-common/archive/e7580d1db7466e6c8403f7826b7558ea5e99bbfd.zip"],
    )
    maybe(
        git_repository,
        name = "com_google_common_flogger",
        commit = "f6071d2c5cd6c6c4f5fcd9f74bfec4ca972b0423",
        # TODO(schroederc): remove usage of fork once https://github.com/google/flogger/pull/37 is closed
        remote = "https://github.com/schroederc/flogger",
    )

    maybe(
        maven_jar,
        name = "com_google_code_gson_gson",
        artifact = "com.google.code.gson:gson:2.8.5",
        sha1 = "f645ed69d595b24d4cf8b3fbb64cc505bede8829",
    )

    maybe(
        maven_jar,
        name = "com_google_guava_guava",
        artifact = "com.google.guava:guava:25.1-jre",
        sha1 = "6c57e4b22b44e89e548b5c9f70f0c45fe10fb0b4",
    )

    maybe(
        maven_jar,
        name = "com_google_re2j_re2j",
        artifact = "com.google.re2j:re2j:1.2",
        sha1 = "4361eed4abe6f84d982cbb26749825f285996dd2",
    )

    maybe(
        maven_jar,
        name = "com_google_code_findbugs_jsr305",
        artifact = "com.google.code.findbugs:jsr305:3.0.1",
        sha1 = "f7be08ec23c21485b9b5a1cf1654c2ec8c58168d",
    )

    maybe(
        maven_jar,
        name = "com_google_errorprone_error_prone_annotations",
        artifact = "com.google.errorprone:error_prone_annotations:2.3.1",
        sha1 = "a6a2b2df72fd13ec466216049b303f206bd66c5d",
    )

    maybe(
        maven_jar,
        name = "org_ow2_asm_asm",
        artifact = "org.ow2.asm:asm:6.0",
        sha1 = "bc6fa6b19424bb9592fe43bbc20178f92d403105",
    )

def _go_dependencies():
    maybe(
        http_archive,
        name = "io_bazel_rules_go",
        sha256 = "5f3b0304cdf0c505ec9e5b3c4fc4a87b5ca21b13d8ecc780c97df3d1809b9ce6",
        urls = ["https://github.com/bazelbuild/rules_go/releases/download/0.15.1/rules_go-0.15.1.tar.gz"],
    )

def kythe_dependencies():
    """Defines external repositories for Kythe dependencies.

    Call this once in your WORKSPACE file to load all @io_kythe dependencies.
    """
    _cc_dependencies()
    _go_dependencies()
    _java_dependencies()

    # proto_library, cc_proto_library, and java_proto_library rules implicitly
    # depend on @com_google_protobuf for protoc and proto runtimes.
    #
    # N.B. We have a near-clone of the protobuf BUILD file overriding upstream so
    # that we can set the unexported config variable to enable zlib. Without this,
    # protobuf silently yields link errors.
    # The most recent release (v3.6.0.1) lacks an fix for
    # https://github.com/google/protobuf/issues/4771, which we rely on in tests.
    # TODO(shahms): Update to the first release to include that fix.
    maybe(
        http_archive,
        name = "com_google_protobuf",
        build_file = "@io_kythe//third_party:protobuf.BUILD",
        sha256 = "08608786f26c2ae4e5ff854560289779314b60179b5df824836303e2c0fae407",
        strip_prefix = "protobuf-964201af37f8a0009440a52a30a66317724a52c3",
        urls = ["https://github.com/google/protobuf/archive/964201af37f8a0009440a52a30a66317724a52c3.zip"],
    )

    maybe(
        http_archive,
        name = "bazel_skylib",
        sha256 = "ca4e3b8e4da9266c3a9101c8f4704fe2e20eb5625b2a6a7d2d7d45e3dd4efffd",
        strip_prefix = "bazel-skylib-0.5.0",
        urls = ["https://github.com/bazelbuild/bazel-skylib/archive/0.5.0.zip"],
    )
