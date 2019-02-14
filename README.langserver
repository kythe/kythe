# Running Kythe Langserver

Note: Below only tested for serving Go code.

## Building

Build the binary, then place it somewhere on your path.

```
bazel build kythe/go/languageserver/...
cp -f bazel-bin/kythe/go/languageserver/kythe_languageserver ~/bin/
```

## Preparing vim-lsp

Put the following snippet in your `.vimrc`:

```
if executable('kythe_languageserver')
    au User lsp_setup call lsp#register_server({
        \ 'name': 'kythe_languageserver',
        \ 'cmd': {server_info->['kythe_languageserver']},
        \ 'whitelist': ['go'],
        \ })
endif
```

## Prepare and serve a Kythe index

This is a complicated topic, but in a nutshell:

```
# index.sh
# Argument to script: bazel target (for example //kythe/go/...).
GS=/tmp/kgs
TAB=/tmp/ktab
ENTRIES=/tmp/entries
UI=/opt/kythe-v0.0.29/web/ui

rm -rf $GS $TAB bazel-out/k8-fastbuild/extra_actions

# Extract go compilations.
bazel build $1 --experimental_action_listener kythe/go/extractors/cmd/bazel:extract_kzip_go

# Write index entries to graphstore.
bazel-bin/kythe/go/indexer/cmd/go_indexer/go_indexer -code $(find bazel-out/k8-fastbuild/extra_actions -name '*.kzip' | xargs -n1 readlink -f) > $ENTRIES

# Prepare serving tables.
# Need to use Beam pipeline, the legacy one won't serve documentation properly.
# See https://github.com/kythe/kythe/issues/3412.
bazel-bin/kythe/go/serving/tools/write_tables/write_tables --entries $ENTRIES --experimental_beam_pipeline --out $TAB

# Serve.
bazel-bin/kythe/go/serving/tools/http_server/http_server --serving_table $TAB --listen 0.0.0.0:8080 --public_resources $UI
```

## Augment the local-to-vname mapping

If needed, tune `.kythe_settings.json`. The Kythe repo now contains a sensible
default for `//kythe/go/...`.

Generally, strive to make the corpus+root prefixes unambigous during your
extraction (can be controlled by vnames.json, see
https://github.com/kythe/kythe/pull/3394).

Note that matching using `.kythe_settings.json` is bidirectional and stops
at the first match. If your prefixes are ambiguous (for example due to having
both empty and non-empty `root` in the same corpus), pay extra attention to the
ordering.

## Use vim-lsp

Start up vim on a `go` source file. Use `:LspHover` to get doc and type info
about the entity under the cursor, `:LspDefinition` to jump to def, `:LspReferences`
to find all backreferences.

Check the langserver status with `:LspStatus`.

To debug the langserver output, execute:

```
tail $(lsof -p $(ps ax | grep kythe_languageserver | grep -v grep | awk '{print $1}') | grep 'log$' | awk '{print $NF}')
```

## Finding references and navigating virtual files.

By using above procedure you can navigate the physical source files. But
generated sources and sources pulled from external workspaces, such as github
repos, won't be navigable.  Moreover, when you look for references on an
entity, and such a reference resides in a non-physical file, `vim-lsp` will
fail to open the file (likely to get snippets) and not display any results.

But worry not! You can use the `kythefs` tool to mount the files in a Kythe
index using a virtual filesystem. After installing `fuse`, the user-space
filesystem utility, for your distribution:

```
bazel build kythe/go/serving/tools/kythefs
# Will block until unmounted with `fusermount -u vfs`
./bazel-bin/kythe/go/serving/tools/kythefs/kythefs --mountpoint vfs
```

The default `.kythe_settings.json` already configures the virtual files to be
mapped to the `vfs` directory in the workspace root, so this should "just work"
(to the extent anything just works usually).

Note: if the `vfs` directory is in your workspace, might hog scanning commands
like `git status` or editors. Probably you can do some trickery by moving both
the `vfs` directory and `.kythe_settings.json` one level up.
