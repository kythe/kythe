def _lein_repository(repository_ctx):
    url = "https://raw.githubusercontent.com/technomancy/leiningen/%s/bin/lein" % (repository_ctx.attr.version,)
    repository_ctx.download(
        url = url,
        output = "home/lein.sh.in",
        sha256 = repository_ctx.attr.sha256,
        executable = False,
    )
    repository_ctx.template("home/lein.sh", "home/lein.sh.in", {"LEIN_JVM_OPTS": "JVM_OPTS"})
    repository_ctx.file(
        "leinbuild.sh",
        """#!/bin/sh
abspath() {
  # Compatibility script for OS X which lacks realpath
  if [ ! -z "$(which realpath)" ]; then
    realpath -s "$1"
  else
    [[ $1 = /* ]] && echo "$1" || echo "$PWD/${1#./}"
  fi
}
export LEIN_HOME="$(abspath "$0.runfiles/%s/home")"
export HOME="$LEIN_HOME",
export JVM_OPTS="-Duser.home=$LEIN_HOME"
PROJECT="$(dirname "${1:?Missing project.cls path}")"
EXECROOT="$PWD"
cd "$PROJECT"
"$LEIN_HOME/lein.sh" -o cljsbuild once prod
"$LEIN_HOME/lein.sh" -o licenses | sort > licenses.txt
cd "$EXECROOT"
mv "$PROJECT"/resources/public/js/main.js "${2:?Missing main.js output path}"
mv "$PROJECT"/licenses.txt "${3:?Missing licenses.txt output path}"
""" % repository_ctx.name,
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
    repository_ctx.symlink(
        repository_ctx.path(Label("@//kythe/web/ui:project.clj")),
        "project.clj",
    )

    # Each of these commands will fetch additional dependencies into LEIN_HOME.
    lein_home = repository_ctx.path("home")
    for cmd in (["deps"], ["cljsbuild", "once"], ["licenses"]):
        repository_ctx.execute(
            ["home/lein.sh"] + cmd,
            environment = {
                "LEIN_HOME": str(lein_home),
                "HOME": str(lein_home),
                "JVM_OPTS": "-Duser.home=" + str(lein_home),
            },
        )

lein_repository = repository_rule(
    implementation = _lein_repository,
    attrs = {
        "version": attr.string(
            default = "2.5.3",
        ),
        "sha256": attr.string(mandatory = True),
    },
)
