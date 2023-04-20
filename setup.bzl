load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")
load("@bazel_tools//tools/build_defs/repo:utils.bzl", "maybe")
load("@bazel_tools//tools/jdk:remote_java_repository.bzl", "remote_java_repository")

def github_archive(name, repo_name, commit, kind = "zip", strip_prefix = "", **kwargs):
    """Defines a GitHub commit-based repository rule."""
    project = repo_name[repo_name.index("/"):]
    http_archive(
        name = name,
        strip_prefix = "{project}-{commit}/{prefix}".format(
            project = project,
            commit = commit,
            prefix = strip_prefix,
        ),
        urls = [u.format(commit = commit, repo_name = repo_name, kind = kind) for u in [
            "https://mirror.bazel.build/github.com/{repo_name}/archive/{commit}.{kind}",
            "https://github.com/{repo_name}/archive/{commit}.{kind}",
        ]],
        **kwargs
    )

def kythe_rule_repositories():
    """Defines external repositories for Kythe Bazel rules.

    These repositories must be loaded before calling external.bzl%kythe_dependencies.
    """
    maybe(
        http_archive,
        name = "bazel_skylib",
        urls = [
            "https://github.com/bazelbuild/bazel-skylib/releases/download/1.3.0/bazel-skylib-1.3.0.tar.gz",
            "https://mirror.bazel.build/github.com/bazelbuild/bazel-skylib/releases/download/1.3.0/bazel-skylib-1.3.0.tar.gz",
        ],
        sha256 = "74d544d96f4a5bb630d465ca8bbcfe231e3594e5aae57e1edbf17a6eb3ca2506",
    )

    maybe(
        http_archive,
        name = "io_bazel_rules_go",
        sha256 = "ae013bf35bd23234d1dea46b079f1e05ba74ac0321423830119d3e787ec73483",
        urls = [
            "https://mirror.bazel.build/github.com/bazelbuild/rules_go/releases/download/v0.36.0/rules_go-v0.36.0.zip",
            "https://github.com/bazelbuild/rules_go/releases/download/v0.36.0/rules_go-v0.36.0.zip",
        ],
    )

    maybe(
        http_archive,
        name = "rules_cc",
        urls = ["https://github.com/bazelbuild/rules_cc/releases/download/0.0.6/rules_cc-0.0.6.tar.gz"],
        sha256 = "3d9e271e2876ba42e114c9b9bc51454e379cbf0ec9ef9d40e2ae4cec61a31b40",
        strip_prefix = "rules_cc-0.0.6",
    )

    maybe(
        http_archive,
        name = "rules_java",
        urls = [
            "https://mirror.bazel.build/github.com/bazelbuild/rules_java/releases/download/5.5.0/rules_java-5.5.0.tar.gz",
            "https://github.com/bazelbuild/rules_java/releases/download/5.5.0/rules_java-5.5.0.tar.gz",
        ],
        sha256 = "bcfabfb407cb0c8820141310faa102f7fb92cc806b0f0e26a625196101b0b57e",
    )

    maybe(
        http_archive,
        name = "rules_proto",
        sha256 = "e4fe70af52135d2ee592a07f916e6e1fc7c94cf8786c15e8c0d0f08b1fe5ea16",
        strip_prefix = "rules_proto-97d8af4dc474595af3900dd85cb3a29ad28cc313",
        url = "https://github.com/bazelbuild/rules_proto/archive/97d8af4dc474595af3900dd85cb3a29ad28cc313.zip",
    )

    maybe(
        http_archive,
        name = "bazel_gazelle",
        sha256 = "448e37e0dbf61d6fa8f00aaa12d191745e14f07c31cabfa731f0c8e8a4f41b97",
        urls = [
            "https://mirror.bazel.build/github.com/bazelbuild/bazel-gazelle/releases/download/v0.28.0/bazel-gazelle-v0.28.0.tar.gz",
            "https://github.com/bazelbuild/bazel-gazelle/releases/download/v0.28.0/bazel-gazelle-v0.28.0.tar.gz",
        ],
    )

    maybe(
        http_archive,
        name = "build_bazel_rules_nodejs",
        sha256 = "dd7ea7efda7655c218ca707f55c3e1b9c68055a70c31a98f264b3445bc8f4cb1",
        urls = ["https://github.com/bazelbuild/rules_nodejs/releases/download/3.2.3/rules_nodejs-3.2.3.tar.gz"],
    )

    maybe(
        http_archive,
        name = "rules_jvm_external",
        sha256 = "8c3b207722e5f97f1c83311582a6c11df99226e65e2471086e296561e57cc954",
        strip_prefix = "rules_jvm_external-5.1",
        urls = ["https://github.com/bazelbuild/rules_jvm_external/releases/download/5.1/rules_jvm_external-5.1.tar.gz"],
    )

    maybe(
        http_archive,
        name = "aspect_rules_ts",
        sha256 = "6406905c5f7c5ca6dedcca5dacbffbf32bb2a5deb77f50da73e7195b2b7e8cbc",
        strip_prefix = "rules_ts-1.0.5",
        url = "https://github.com/aspect-build/rules_ts/archive/refs/tags/v1.0.5.tar.gz",
    )

    maybe(
        http_archive,
        name = "aspect_rules_jasmine",
        sha256 = "0357d45b5dba77004931db83ced43c6c432eee658a51d1876a9f2b57838e4080",
        strip_prefix = "rules_jasmine-0.2.1",
        url = "https://github.com/aspect-build/rules_jasmine/archive/refs/tags/v0.2.1.tar.gz",
    )

    maybe(
        http_archive,
        name = "rules_python",
        sha256 = "e5470e92a18aa51830db99a4d9c492cc613761d5bdb7131c04bd92b9834380f6",
        strip_prefix = "rules_python-4b84ad270387a7c439ebdccfd530e2339601ef27",
        urls = [
            "https://mirror.bazel.build/github.com/bazelbuild/rules_python/archive/4b84ad270387a7c439ebdccfd530e2339601ef27.tar.gz",
            "https://github.com/bazelbuild/rules_python/archive/4b84ad270387a7c439ebdccfd530e2339601ef27.tar.gz",
        ],
    )

    maybe(
        http_archive,
        name = "rules_rust",
        sha256 = "05e15e536cc1e5fd7b395d044fc2dabf73d2b27622fbc10504b7e48219bb09bc",
        urls = [
            "https://mirror.bazel.build/github.com/bazelbuild/rules_rust/releases/download/0.8.1/rules_rust-v0.8.1.tar.gz",
            "https://github.com/bazelbuild/rules_rust/releases/download/0.8.1/rules_rust-v0.8.1.tar.gz",
        ],
    )

    maybe(
        http_archive,
        name = "bazelruby_rules_ruby",
        strip_prefix = "rules_ruby-0.4.1",
        sha256 = "abfc2758cc379e0ff0eb9824e3b507c1633d4c8f99f24735aef63c7180be50f0",
        urls = [
            "https://github.com/bazelruby/rules_ruby/archive/v0.4.1.zip",
        ],
        patches = [
            "@io_kythe//third_party:rules_ruby_allow_empty.patch",
        ],
        patch_args = ["-p1"],
    )

    maybe(
        http_archive,
        name = "rules_foreign_cc",
        sha256 = "e60cfd0a8426fa4f5fd2156e768493ca62b87d125cb35e94c44e79a3f0d8635f",
        strip_prefix = "rules_foreign_cc-0.2.0",
        url = "https://github.com/bazelbuild/rules_foreign_cc/archive/0.2.0.zip",
    )

    maybe(
        github_archive,
        repo_name = "llvm/llvm-project",
        commit = "357da2f5979647bc0f0589506c165e941b05ace3",
        sha256 = "003c1f3f22ddbb066e5b1dd89b4a31931731e9514972ba7ae0aea02c43bf0db2",
        name = "llvm-raw",
        build_file_content = "#empty",
        patch_args = ["-p1"],
        patches = ["@io_kythe//third_party:llvm-bazel-glob.patch"],
    )

    maybe(
        github_archive,
        repo_name = "hedronvision/bazel-compile-commands-extractor",
        name = "hedron_compile_commands",
        commit = "d6734f1d7848800edc92de48fb9d9b82f2677958",
        sha256 = "0dfe793b5779855cf73b3ee9f430e00225f51f38c70555936d4dd6f1b3c65e66",
    )

    # proto_library, cc_proto_library, and java_proto_library rules implicitly
    # depend on @com_google_protobuf for protoc and proto runtimes.
    maybe(
        http_archive,
        name = "com_google_protobuf",
        sha256 = "1ff680568f8e537bb4be9813bac0c1d87848d5be9d000ebe30f0bc2d7aabe045",
        strip_prefix = "protobuf-22.2",
        urls = [
            "https://mirror.bazel.build/github.com/protocolbuffers/protobuf/releases/download/v22.2/protobuf-22.2.tar.gz",
            "https://github.com/protocolbuffers/protobuf/releases/download/v22.2/protobuf-22.2.tar.gz",
        ],
        repo_mapping = {"@zlib": "@net_zlib"},
    )

def remote_jdk20_repos():
    """Imports OpenJDK 20 repositories."""
    maybe(
        remote_java_repository,
        name = "remotejdk20_linux",
        target_compatible_with = [
            "@platforms//os:linux",
            "@platforms//cpu:x86_64",
        ],
        sha256 = "0386418db7f23ae677d05045d30224094fc13423593ce9cd087d455069893bac",
        strip_prefix = "zulu20.28.85-ca-jdk20.0.0-linux_x64",
        urls = [
            "https://mirror.bazel.build/cdn.azul.com/zulu/bin/zulu20.28.85-ca-jdk20.0.0-linux_x64.tar.gz",
            "https://cdn.azul.com/zulu/bin/zulu20.28.85-ca-jdk20.0.0-linux_x64.tar.gz",
        ],
        version = "20",
    )

    maybe(
        remote_java_repository,
        name = "remotejdk20_macos",
        target_compatible_with = [
            "@platforms//os:macos",
            "@platforms//cpu:x86_64",
        ],
        sha256 = "fde6cc17a194ea0d9b0c6c0cb6178199d8edfc282d649eec2c86a9796e843f86",
        strip_prefix = "zulu20.28.85-ca-jdk20.0.0-macosx_x64",
        urls = [
            "https://mirror.bazel.build/cdn.azul.com/zulu/bin/zulu20.28.85-ca-jdk20.0.0-macosx_x64.tar.gz",
            "https://cdn.azul.com/zulu/bin/zulu20.28.85-ca-jdk20.0.0-macosx_x64.tar.gz",
        ],
        version = "20",
    )

    maybe(
        remote_java_repository,
        name = "remotejdk20_macos_aarch64",
        target_compatible_with = [
            "@platforms//os:macos",
            "@platforms//cpu:aarch64",
        ],
        sha256 = "a2eff6a940c2df3a2352278027e83f5959f34dcfc8663034fe92be0f1b91ce6f",
        strip_prefix = "zulu20.28.85-ca-jdk20.0.0-macosx_aarch64",
        urls = [
            "https://mirror.bazel.build/cdn.azul.com/zulu/bin/zulu20.28.85-ca-jdk20.0.0-macosx_aarch64.tar.gz",
            "https://cdn.azul.com/zulu/bin/zulu20.28.85-ca-jdk20.0.0-macosx_aarch64.tar.gz",
        ],
        version = "20",
    )
    maybe(
        remote_java_repository,
        name = "remotejdk20_win",
        target_compatible_with = [
            "@platforms//os:windows",
            "@platforms//cpu:x86_64",
        ],
        sha256 = "ac5f6a7d84dbbb0bb4d376feb331cc4c49a9920562f2a5e85b7a6b4863b10e1e",
        strip_prefix = "zulu20.28.85-ca-jdk20.0.0-win_x64",
        urls = [
            "https://mirror.bazel.build/cdn.azul.com/zulu/bin/zulu20.28.85-ca-jdk20.0.0-win_x64.zip",
            "https://cdn.azul.com/zulu/bin/zulu20.28.85-ca-jdk20.0.0-win_x64.zip",
        ],
        version = "20",
    )
