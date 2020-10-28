## Bazel C++ extractor

An extractor that builds index files from a Bazel-based project. Extractor
builds a kzip file for each `cc_library` and `cc_binary` in the project. This
extractor based on Bazel
[action_listener](https://docs.bazel.build/versions/master/be/extra-actions.html)
rule.

#### Usage

These instructions assume the kythe is installed in /opt/kythe. If not, follow
[installation](http://kythe.io/getting-started) instructions.

Currently Bazel doesn't have a convenient way to use action listener outside of
a project. So first you need to add `action_listener` to the root BUILD file of
the project.

```python
# Extra action invokes /opt/kythe/extractors/bazel_cxx_extractor
extra_action(
    name = "extractor",
    cmd = ("/opt/kythe/extractors/bazel_cxx_extractor " +
           "$(EXTRA_ACTION_FILE) $(output $(ACTION_ID).cxx.kzip) $(location :vnames.json)"),
    data = [":vnames.json"],
    out_templates = ["$(ACTION_ID).cxx.kzip"],
)

action_listener(
    name = "extract_cxx",
    extra_actions = [":extractor"],
    mnemonics = ["CppCompile"],
    visibility = ["//visibility:public"],
)
```

Extractor requires a `vnames.json` file that tells extractor how to translate
certain filepaths. For example in Bazel project java files often stored in
`java` and `javatests` directories. But filepath like
`java/com/some/domain/Foo.java` should be extracted as
`com/some/domain/Foo.java` with the `java` prefix omitted. `vnames.json` file
tells extractor how to rename certain filepaths during extraction. As an example
check `vnames.json` from Kythe repo:
[link](https://github.com/kythe/kythe/blob/master/kythe/data/vnames.json).

```shell
cd $YOUR_BAZEL_PROJECT
# As example copy vnames.json from Kythe repo. But you should change it for your project
# later.
curl https://raw.githubusercontent.com/kythe/kythe/master/kythe/data/vnames.json > vnames.json
```

Run extractor:

```shell
# run on all targets
bazel test --experimental_action_listener=:extract_cxx  //...

# run on specific target (e.g. some cc_binary)
bazel test --experimental_action_listener=:extract_cxx  //java/some/folder:foo
```

Extracted kzip files will be in
`bazel-out/local-fastbuild/extra_actions/extractor` folder. One kzip file per
target.

```shell
find -L bazel-out -name '*cxx.kzip'
```

#### Development

Building extractor from sources:

```shell
export KYTHE_PROJECT=path/to/kythe/repo/directory
cd $KYTHE_PROJECT
bazel build //kythe/cxx/extractor:cxx_extractor_bazel
```

Freshly built extractor will be in folder
`bazel-bin/kythe/cxx/extractor/cxx_extractor_bazel
/opt/kythe/extractors/bazel_cxx_extractor`. Follow instructions of above but
`extra_action` replace `/opt/kythe/extractors/bazel_cxx_extractor` with the path
to the freshly built exractor.
