def _lein_repository(repository_ctx):
    url = "https://raw.githubusercontent.com/technomancy/leiningen/%s/bin/lein" % (repository_ctx.attr.version,)
    lein = repository_ctx.download(
        url = url,
        output = "home/lein.sh",
        sha256 = repository_ctx.attr.sha256,
        executable = True,
    )
    repository_ctx.symlink(
        repository_ctx.path(Label("@//kythe/web/ui:project.clj")),
        "project.clj",
    )

    # Each of these commands will fetch additional dependencies into LEIN_HOME.
    for cmd in (["deps"], ["cljsbuild", "once"], ["licenses"]):
        repository_ctx.execute(
            ["home/lein.sh"] + cmd,
            environment = {"LEIN_HOME": str(repository_ctx.path("home"))},
        )
    repository_ctx.file(
        "leinbuild.sh",
        "\n".join([
            "#!/bin/sh",
            "export LEIN_HOME=\"%s\"" % (repository_ctx.path("home"),),
            "PROJECT=\"$(dirname \"${1:?Missing project.cls path}\")\"",
            "EXECROOT=\"$PWD\"",
            "cd \"$PROJECT\"",
            "\"$LEIN_HOME/lein.sh\" cljsbuild once prod",
            "\"$LEIN_HOME/lein.sh\" licenses | sort > licenses.txt",
            "cd \"$EXECROOT\"",
            "mv \"$PROJECT\"/resources/public/js/main.js \"${2:?Missing main.js output path}\"",
            "mv \"$PROJECT\"/licenses.txt \"${3:?Missing licenses.txt output path}\"",
        ]),
        executable = True,
    )
    repository_ctx.file(
        "WORKSPACE",
        "workspace(name = \"org_leiningen\")\n",
    )
    repository_ctx.file(
        "BUILD",
        "\n".join(
            [
                "package(default_visibility = [\"//visibility:public\"])",
                "licenses([\"reciprocal\"])  # EPL 1.0",
                "sh_binary(name = \"leinbuild\", srcs = [\"leinbuild.sh\"], data = glob([\"home/**/*\"]))",
            ],
        ),
    )

lein_repository = repository_rule(
    implementation = _lein_repository,
    attrs = {
        "version": attr.string(
            default = "2.5.1",
        ),
        "sha256": attr.string(mandatory = True),
    },
)
