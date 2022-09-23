---
layout: page
title: Examples
permalink: /examples/
order: 10
---

* toc
{:toc}

This document assumes that the latest release archive from
[https://github.com/kythe/kythe/releases](https://github.com/kythe/kythe/releases)
has been unpacked into /opt/kythe/.  See /opt/kythe/README.md for more
information.

## Extracting Compilations

{% highlight bash %}
# Generate Kythe protobuf sources
bazel build //kythe/proto:all

# Environment variables common to Kythe extractors
export KYTHE_ROOT_DIRECTORY="$PWD"                        # Root of source code corpus
export KYTHE_OUTPUT_DIRECTORY="/tmp/kythe.compilations/"  # Output directory
export KYTHE_VNAMES="$PWD/kythe/data/vnames.json"         # Optional: VNames configuration

mkdir -p "$KYTHE_OUTPUT_DIRECTORY"

# Extract a Java compilation
# java -Xbootclasspath/p:third_party/javac/javac*.jar \
#   com.google.devtools.kythe.extractors.java.standalone.Javac8Wrapper \
#   <javac_arguments>
java -Xbootclasspath/p:third_party/javac/javac*.jar \
  -jar /opt/kythe/extractors/javac_extractor.jar \
  com.google.devtools.kythe.extractors.java.standalone.Javac8Wrapper \
  kythe/java/com/google/devtools/kythe/platform/kzip/*.java

# Extract a C++ compilation
# /opt/kythe/extractors/cxx_extractor <arguments>
/opt/kythe/extractors/cxx_extractor -x c++ kythe/cxx/common/scope_guard.h
{% endhighlight %}

## Extracting Compilations using Bazel

Kythe uses Bazel to build itself and has implemented Bazel
[action_listener](https://docs.bazel.build/versions/master/be/extra-actions.html#action_listener)s
that use Kythe's Java and C++ extractors.  This effectively allows Bazel to
extract each compilation as it is run during the build.

### Extracting the Kythe repository

Add the flag
`--experimental_action_listener=@io_kythe//kythe/extractors:extract_kzip_java`
to make Bazel extract Java compilations and
`--experimental_action_listener=@io_kythe//kythe/extractors:extract_kzip_cxx` to do the
same for C++.

{% highlight bash %}
# Extract all Java/C++ compilations in Kythe
bazel build -k \
  --experimental_action_listener=@io_kythe//kythe/extractors:extract_kzip_java \
  --experimental_action_listener=@io_kythe//kythe/extractors:extract_kzip_cxx \
  --experimental_extra_action_top_level_only \
  //kythe/cxx/... //kythe/java/...

# Find the extracted .kzip files
find -L bazel-out -name '*.kzip'
{% endhighlight %}

### Extracting other Bazel based repositories

You can use the Kythe release to extract compilations from other Bazel based
repositories.

{% highlight bash%}
# Download and unpack the latest Kythe release
wget -O /tmp/kythe.tar.gz \
    https://github.com/kythe/kythe/releases/download/$KYTHE_VERSION/kythe-$KYTHE_VERSION.tar.gz
tar --no-same-owner -xvzf /tmp/kythe.tar.gz --directory /opt
echo 'KYTHE_DIR=/opt/kythe-$KYTHE_VERSION' >> $BASH_ENV

# Build the repository with extraction enabled
bazel --bazelrc=$KYTHE_DIR/extractors.bazelrc \
    build --override_repository kythe_release=$KYTHE_DIR \
    //...
{% endhighlight %}

## runextractor tool

`runextractor` is a generic extraction tool that works with any build system capable of emitting a [compile_commands.json](https://clang.llvm.org/docs/JSONCompilationDatabase.html) file. `runextractor` invokes an extractor for each compilation action listed in compile_commands.json and generates a kzip in the output directory for each.

Build systems capable of emitting a `compile_commands.json` include CMake, Ninja, [gn](https://gn.googlesource.com/gn/+/master/docs/reference.md), waf, and others.


### runextractor configuration

`runextractor` is configured via a set of environment variables:

*   `KYTHE_ROOT_DIRECTORY`: The absolute path for file input to be extracted.
    This is generally the root of the repository. All files extracted will be
    stored relative to this path.
*   `KYTHE_OUTPUT_DIRECTORY`: The absolute path for storing output.
*   `KYTHE_CORPUS`: The corpus label for extracted files.


### Extracting from a compile_commands.json file

This example uses Ninja, but the first step can be adapted for others.

1. Begin by building your project with compile_commands.json enabled. For ninja, the command is `ninja -t compdb > compile_commands.json`
2. Set environment variables - see above section.
3. Invoke runextractor: `runextractor compdb -extractor /opt/kythe/extractors/cxx_extractor`
4. If successful, the output directory should contain one kzip for each compilation action. An optional last step is to merge these into one kzip with `/opt/kythe/tools/kzip merge --output $KYTHE_OUTPUT_DIRECTORY/merged.kzip $KYTHE_OUTPUT_DIRECTORY/*.kzip`.

### Extracting CMake based repositories

The `runextractor` tool has a convenience subcommand for cmake-based repositories that first invokes CMake to generate a compile_commands.json, then processes the listed compilation actions. However the same result could be achieved by invoking CMake manually, then using the generic `runextractor compdb` command.

**These instructions assume your environment is already set up to successfully
run cmake for your repository**.

```shell
$ export KYTHE_ROOT_DIRECTORY="/absolute/path/to/repo/root"
$ export KYTHE_CORPUS="github.com/myproject/myrepo"

$ export KYTHE_OUTPUT_DIRECTORY="/tmp/kythe-output"
$ mkdir -p "$KYTHE_OUTPUT_DIRECTORY"

# $CMAKE_ROOT_DIRECTORY is passed into the -sourcedir flag. This value should be
# the directory that contains the top-level CMakeLists.txt file. In many
# repositories this path is the same as $KYTHE_ROOT_DIRECTORY.
$ export CMAKE_ROOT_DIRECTORY="/absolute/path/to/cmake/root"

$ /opt/kythe/tools/runextractor cmake \
    -extractor=/opt/kythe/extractors/cxx_extractor \
    -sourcedir=$CMAKE_ROOT_DIRECTORY
```

## Extracting Gradle based repositories

1. Install compiler wrapper

    Extraction works by intercepting all calls to `javac` and saving the compiler arguments and inputs to a "compilation unit", which is stored in a .kzip file. We have a javac-wrapper.sh script that forwards javac calls to the java extractor and then calls javac. Add this to the end of your project's build.gradle:

    ```groovy
    allprojects {
      gradle.projectsEvaluated {
        tasks.withType(JavaCompile) {
          options.fork = true
          options.forkOptions.executable = '/opt/kythe/extractors/javac-wrapper.sh'
        }
      }
    }
    ```

2. VName configuration

    Next, you will need to create a vnames.json mapping file, which tells the
    extractor how to assign vnames to files based on their paths. A basic vnames
    config for a gradle project looks like:

    ```json
    [
      {
        "pattern": "(build/[^/]+)/(.*)",
        "vname": {
          "corpus": "MY_CORPUS",
          "path": "@2@",
          "root": "@1@"
        }
      },
      {
        "pattern": ".*/.gradle/caches/(.*)",
        "vname": {
          "corpus": "MY_CORPUS",
          "path": "@1@",
          "root": ".gradle/caches"
        }
      },
      {
        "pattern": "(.*)",
        "vname": {
          "corpus": "MY_CORPUS",
          "path": "@1@"
        }
      }
    ]
    ```

    (note: change "MY_CORPUS" to the actual corpus for your project)

    You can test your vname config using the `vnames` command line tool. For example:

    ```shell
    bazel build //kythe/go/util/tools/vnames

    echo "some/test/path.java" | ./bazel-bin/kythe/go/util/tools/vnames/vnames apply-rules --rules vnames.json
    > {
    >   "corpus": "MY_CORPUS",
    >   "path": "some/test/path.java"
    > }
    ```


3. Extraction

    ```shell
    # note: you may want to use a different javac depending on your install
    export REAL_JAVAC="/usr/bin/javac"
    export JAVA_HOME="$(readlink -f $REAL_JAVAC | sed 's:/bin/javac::')"
    export JAVAC_EXTRACTOR_JAR="/opt/kythe/extractors/javac_extractor.jar"

    export KYTHE_VNAMES="$PWD/vnames.json"

    export KYTHE_ROOT_DIRECTORY="$PWD" # paths in the compilation unit will be made relative to this
    export KYTHE_OUTPUT_DIRECTORY="/tmp/extracted_gradle_project"
    mkdir -p "$KYTHE_OUTPUT_DIRECTORY"

    ./gradlew clean build -x test -Dno_werror=true

    # merge all kzips into one
    /opt/kythe/tools/kzip merge --output $KYTHE_OUTPUT_DIRECTORY/merged.kzip $KYTHE_OUTPUT_DIRECTORY/*.kzip
    ```

4. Examine results

    If extraction was successful, the final kzip should be at `$KYTHE_OUTPUT_DIRECTORY/merged.kzip`. The `kzip` tool can be used to inspect the result.

    ```shell
    $ kzip info --input merged.kzip | jq . # view summary information
    $ kzip view merged.kzip | jq .         # view all compilation units in the kzip
    ```


## Extracting projects built with `make`

Projects built with make can be extracted by substituting the C/C++ compiler with a wrapper script that invokes both Kythe's cxx_extractor binary and the actual C/C++ compiler.

Given a simple example project:

```c++
# main.cc

#include <iostream>

int main(int argc, char** argv) {
    std::cout << "Hello" << std::endl;
}
```

```shell
# makefile

all: bin

bin: main.cc
  $(CXX) main.cc -o bin

```

```shell
# cxx_wrapper.sh
#!/bin/bash -e

$KYTHE_RELEASE_DIR/extractors/cxx_extractor "$@" &
/usr/bin/c++ "$@"
```

Extraction is done by setting the `CXX` make variable as well as some environment variables that configure `cxx_extractor`.

```shell
# directory where kythe release has been installed
export KYTHE_RELEASE_DIR=/opt/kythe-v0.0.50

# parameters for cxx_extractor
export KYTHE_CORPUS=mycorpus
export KYTHE_ROOT_DIRECTORY="$PWD"
export KYTHE_OUTPUT_DIRECTORY=/tmp/extract

export CXX="cxx_wrapper.sh"

mkdir -p "$KYTHE_OUTPUT_DIRECTORY"

make
```

If all goes well, this will populate `$KYTHE_OUTPUT_DIRECTORY` with kzip files, one for each compiler invocation. These files can be inspected with the `kzip` tool distributed as part of the kythe release. For example `kzip view $KYTHE_OUTPUT_DIRECTORY/some.file.kzip | jq`.


## Indexing Compilations

All Kythe indexers analyze compilations emitted from
[extractors](#extracting-compilations) as either a
[.kzip file]({{site.baseurl}}/docs/kythe-kzip.html).  The indexers will then
emit a [delimited
stream]({{site.data.development.source_browser}}/kythe/go/platform/delimited/delimited.go)
of [entry protobufs]({{site.baseurl}}/docs/kythe-storage.html#_entry) that can
then be stored in a [GraphStore]({{site.baseurl}}/docs/kythe-storage.html).

{% highlight bash %}
# Indexing a C++ compilation
# /opt/kythe/indexers/cxx_indexer --ignore_unimplemented <kzip-file> > entries
/opt/kythe/indexers/cxx_indexer --ignore_unimplemented \
  .kythe_compilations/c++/kythe_cxx_indexer_cxx_libIndexerASTHooks.cc.c++.kzip > entries

# Indexing a Java compilation
# java -Xbootclasspath/p:third_party/javac/javac*.jar \
#   com.google.devtools.kythe.analyzers.java.JavaIndexer \
#   <kzip-file> > entries
java -Xbootclasspath/p:third_party/javac/javac*.jar \
  com.google.devtools.kythe.analyzers.java.JavaIndexer \
  $PWD/.kythe_compilations/java/kythe_java_com_google_devtools_kythe_analyzers_java_analyzer.java.kzip > entries

# View indexer's output entry stream as JSON
/opt/kythe/tools/entrystream --write_format=json < entries

# Write entry stream into a GraphStore
/opt/kythe/tools/write_entries --graphstore leveldb:/tmp/gs < entries
{% endhighlight %}

## Indexing the Kythe Repository

{% highlight bash %}
mkdir -p .kythe_{graphstore,compilations}
# .kythe_serving is the output directory for the resulting Kythe serving tables
# .kythe_graphstore is the output directory for the resulting Kythe GraphStore
# .kythe_compilations will contain the intermediary .kzip file for each
#   indexed compilation

# Produce the .kzip files for each compilation in the Kythe repo
./kythe/extractors/bazel/extract.sh "$PWD" .kythe_compilations

# Index the compilations, producing a GraphStore containing a Kythe index
bazel build //kythe/release:docker
docker run --rm \
  -v "${PWD}:/repo" \
  -v "${PWD}/.kythe_compilations:/compilations" \
  -v "${PWD}/.kythe_graphstore:/graphstore" \
  google/kythe --index

# Generate corresponding serving tables
/opt/kythe/tools/write_tables --graphstore .kythe_graphstore --out .kythe_serving
{% endhighlight %}

## Using Cayley to explore a GraphStore

Install Cayley if necessary:
[https://github.com/google/cayley/releases](https://github.com/google/cayley/releases)

{% highlight bash %}
# Convert GraphStore to nquads format
bazel run //kythe/go/storage/tools/triples --graphstore /path/to/graphstore | \
  gzip >kythe.nq.gz

cayley repl --dbpath kythe.nq.gz # or cayley http --dbpath kythe.nq.gz
{% endhighlight %}

    // Get all file nodes
    cayley> g.V().Has("/kythe/node/kind", "file").All()

    // Get definition anchors for all record nodes
    cayley> g.V().Has("/kythe/node/kind", "record").Tag("record").In("/kythe/edge/defines").All()

    // Get the file(s) defining a particular node
    cayley> g.V("node_ticket").In("/kythe/edge/defines").Out("/kythe/edge/childof").Has("/kythe/node/kind", "file").All()

## Visualizing Cross-References

As part of Kythe's first release, a sample UI has been made to show Kythe's
basic cross-reference capabilities.  The following command can be run over the
serving table created with the `write_tables` binary (see above).

Note: Version v0.0.30 is the latest version that includes the web UI. If you want a newer Kythe than this, you'll need to build from source.

{% highlight bash %}
# --listen localhost:8080 allows access from only this machine; change to
# --listen :8080 to allow access from any machine
/opt/kythe/tools/http_server \
  --listen localhost:8080 \
  --serving_table .kythe_serving
{% endhighlight %}
