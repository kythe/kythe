load("@bazel_tools//tools/build_defs/repo:git.bzl", "git_repository")
load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

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

    maybe(
        http_archive,
        name = "org_libzip",
        build_file = "@io_kythe//third_party:libzip.BUILD",
        sha256 = "a5d22f0c87a2625450eaa5e10db18b8ee4ef17042102d04c62e311993a2ba363",
        strip_prefix = "libzip-rel-1-5-1",
        urls = [
            # Bazel does not like the official download link at libzip.org,
            # so use the GitHub release tag.
            "https://github.com/nih-at/libzip/archive/rel-1-5-1.zip",
        ],
    )

    maybe(
        http_archive,
        name = "boringssl",  # Must match upstream workspace name.
        # Gitiles creates gzip files with an embedded timestamp, so we cannot use
        # sha256 to validate the archives.  We must rely on the commit hash and https.
        # Commits must come from the master-with-bazel branch.
        url = "https://boringssl.googlesource.com/boringssl/+archive/4be3aa87917b20fedc45fa1fc5b6a2f3738612ad.tar.gz",
    )

    maybe(
        http_archive,
        name = "com_github_tencent_rapidjson",
        build_file = "@io_kythe//third_party:rapidjson.BUILD",
        sha256 = "8e00c38829d6785a2dfb951bb87c6974fa07dfe488aa5b25deec4b8bc0f6a3ab",
        strip_prefix = "rapidjson-1.1.0",
        url = "https://github.com/Tencent/rapidjson/archive/v1.1.0.zip",
    )

    # Make sure to update regularly in accordance with Abseil's principle of live at HEAD
    maybe(
        http_archive,
        name = "com_google_absl",
        sha256 = "84c749757edd12da6188a0629a5d26bff8bba72a123b3aa571f4f5d9a03eaee6",
        strip_prefix = "abseil-cpp-8f612ebb152fb7e05643a2bcf78cb89a8c0641ad",
        url = "https://github.com/abseil/abseil-cpp/archive/8f612ebb152fb7e05643a2bcf78cb89a8c0641ad.zip",
    )

    maybe(
        http_archive,
        name = "com_google_googletest",
        sha256 = "89cebb92b9a7eb32c53e180ccc0db8f677c3e838883c5fbd07e6412d7e1f12c7",
        strip_prefix = "googletest-d175c8bf823e709d570772b038757fadf63bc632",
        url = "https://github.com/google/googletest/archive/d175c8bf823e709d570772b038757fadf63bc632.zip",
    )

    maybe(
        http_archive,
        name = "com_github_gflags_gflags",
        sha256 = "94ad0467a0de3331de86216cbc05636051be274bf2160f6e86f07345213ba45b",
        strip_prefix = "gflags-77592648e3f3be87d6c7123eb81cbad75f9aef5a",
        url = "https://github.com/gflags/gflags/archive/77592648e3f3be87d6c7123eb81cbad75f9aef5a.zip",
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
        native.maven_jar,
        name = "com_google_code_gson_gson",
        artifact = "com.google.code.gson:gson:2.8.5",
        sha1 = "f645ed69d595b24d4cf8b3fbb64cc505bede8829",
    )

    maybe(
        native.maven_jar,
        name = "com_google_guava_guava",
        artifact = "com.google.guava:guava:25.1-jre",
        sha1 = "6c57e4b22b44e89e548b5c9f70f0c45fe10fb0b4",
    )

    maybe(
        native.maven_jar,
        name = "com_google_re2j_re2j",
        artifact = "com.google.re2j:re2j:1.2",
        sha1 = "4361eed4abe6f84d982cbb26749825f285996dd2",
    )

    maybe(
        native.maven_jar,
        name = "com_google_code_findbugs_jsr305",
        artifact = "com.google.code.findbugs:jsr305:3.0.1",
        sha1 = "f7be08ec23c21485b9b5a1cf1654c2ec8c58168d",
    )

    maybe(
        native.maven_jar,
        name = "com_google_errorprone_error_prone_annotations",
        artifact = "com.google.errorprone:error_prone_annotations:2.3.1",
        sha1 = "a6a2b2df72fd13ec466216049b303f206bd66c5d",
    )

    maybe(
        native.maven_jar,
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
    maybe(
        http_archive,
        name = "com_google_protobuf",
        build_file = "@io_kythe//third_party:protobuf.BUILD",
        sha256 = "d7a221b3d4fb4f05b7473795ccea9e05dab3b8721f6286a95fffbffc2d926f8b",
        strip_prefix = "protobuf-3.6.1",
        urls = ["https://github.com/protocolbuffers/protobuf/archive/v3.6.1.zip"],
    )

    maybe(
        http_archive,
        name = "bazel_skylib",
        sha256 = "ca4e3b8e4da9266c3a9101c8f4704fe2e20eb5625b2a6a7d2d7d45e3dd4efffd",
        strip_prefix = "bazel-skylib-0.5.0",
        urls = ["https://github.com/bazelbuild/bazel-skylib/archive/0.5.0.zip"],
    )
