load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")
load("@bazel_tools//tools/build_defs/repo:utils.bzl", "maybe")

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
            "https://github.com/bazelbuild/bazel-skylib/releases/download/1.0.3/bazel-skylib-1.0.3.tar.gz",
            "https://mirror.bazel.build/github.com/bazelbuild/bazel-skylib/releases/download/1.0.3/bazel-skylib-1.0.3.tar.gz",
        ],
        sha256 = "1c531376ac7e5a180e0237938a2536de0c54d93f5c278634818e0efc952dd56c",
    )

    maybe(
        github_archive,
        repo_name = "bazelbuild/rules_go",
        commit = "d0d326e4a5df51b5103dcd332c9e4df06d0bbbf2",
        name = "io_bazel_rules_go",
    )

    maybe(
        http_archive,
        name = "rules_cc",
        sha256 = "ff7876d80cd3f6b8c7a064bd9aa42a78e02096544cca2b22a9cf390a4397a53e",
        strip_prefix = "rules_cc-081771d4a0e9d7d3aa0eed2ef389fa4700dfb23e",
        urls = [
            "https://mirror.bazel.build/github.com/bazelbuild/rules_cc/archive/081771d4a0e9d7d3aa0eed2ef389fa4700dfb23e.tar.gz",
            "https://github.com/bazelbuild/rules_cc/archive/081771d4a0e9d7d3aa0eed2ef389fa4700dfb23e.tar.gz",
        ],
    )

    maybe(
        http_archive,
        name = "rules_java",
        url = "https://github.com/bazelbuild/rules_java/archive/973a93dd2d698929264d1028836f6b9cc60ff817.zip",
        sha256 = "a6cb0dbe343b67c7d4f3f11a68e327acdfc71fee1e17affa4e605129fc56bb15",
        strip_prefix = "rules_java-973a93dd2d698929264d1028836f6b9cc60ff817",
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
        sha256 = "62ca106be173579c0a167deb23358fdfe71ffa1e4cfdddf5582af26520f1c66f",
        urls = [
            "https://mirror.bazel.build/github.com/bazelbuild/bazel-gazelle/releases/download/v0.23.0/bazel-gazelle-v0.23.0.tar.gz",
            "https://github.com/bazelbuild/bazel-gazelle/releases/download/v0.23.0/bazel-gazelle-v0.23.0.tar.gz",
        ],
    )

    maybe(
        http_archive,
        name = "build_bazel_rules_nodejs",
        sha256 = "2b2004784358655f334925e7eadc7ba80f701144363df949b3293e1ae7a2fb7b",
        urls = ["https://github.com/bazelbuild/rules_nodejs/releases/download/5.4.0/rules_nodejs-5.4.0.tar.gz"],
    )

    maybe(
        http_archive,
        name = "rules_jvm_external",
        sha256 = "31701ad93dbfe544d597dbe62c9a1fdd76d81d8a9150c2bf1ecf928ecdf97169",
        strip_prefix = "rules_jvm_external-4.0",
        urls = ["https://github.com/bazelbuild/rules_jvm_external/archive/4.0.zip"],
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
        sha256 = "39458d9bc8f44b23bf963a09c2823b1703b312bcb51d2e8938d640e5b9780180",
        strip_prefix = "rules_rust-1f2e6231de29d8fad8d21486f0d16403632700bf",
        urls = [
            "https://github.com/bazelbuild/rules_rust/archive/1f2e6231de29d8fad8d21486f0d16403632700bf.tar.gz",
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
        commit = "4c564940a14f55d2315d2676b10fea0660ea814a",
        name = "llvm-project-raw",
        build_file_content = "#empty",
        patch_args = ["-p1"],
        patches = ["@io_kythe//third_party:llvm-bazel-glob.patch"],
    )
