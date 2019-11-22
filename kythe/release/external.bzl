load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

def maybe(repo_rule, name, **kwargs):
    """Defines a repository if it does not already exist.
    """
    if name not in native.existing_rules():
        repo_rule(name = name, **kwargs)

def kythe_release_dependencies():
    """Defines external repositories for Kythe dependencies.

    Call this once in your WORKSPACE file to load all @kythe_release dependencies.
    """

    maybe(
        http_archive,
        name = "rules_proto",
        sha256 = "602e7161d9195e50246177e7c55b2f39950a9cf7366f74ed5f22fd45750cd208",
        strip_prefix = "rules_proto-97d8af4dc474595af3900dd85cb3a29ad28cc313",
        urls = ["https://github.com/bazelbuild/rules_proto/archive/97d8af4dc474595af3900dd85cb3a29ad28cc313.tar.gz"],
    )

    maybe(
        http_archive,
        name = "com_google_protobuf",
        sha256 = "2ba20d91341ef88259896a5dfaf55666d11648caa0964342991e30a96b7cd630",
        strip_prefix = "protobuf-3.10.0-rc1",
        urls = ["https://github.com/protocolbuffers/protobuf/archive/v3.10.0-rc1.zip"],
    )

    maybe(
        http_archive,
        name = "com_google_absl",
        sha256 = "c1b570e3d48527c6eb5d8668cd4d2a24b704110700adc0db44b002c058fdf5d0",
        strip_prefix = "abseil-cpp-c6c3c1b498e4ee939b24be59cae29d59c3863be8",
        url = "https://github.com/abseil/abseil-cpp/archive/c6c3c1b498e4ee939b24be59cae29d59c3863be8.zip",
    )
