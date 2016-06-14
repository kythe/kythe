---
layout: page
title: Examples
permalink: /examples/
order: 10
---

* toc
{:toc}

{% comment %}
TODO(schroederc):
 - source -> dot graph examples
 - running Java/C++/Go indexers
{% endcomment %}

This document assumes that the latest release archive from
[https://github.com/google/kythe/releases](https://github.com/google/kythe/releases)
has been unpacked into /opt/kythe/.  See /opt/kythe/README for more information.

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
# java -jar /opt/kythe/extractors/javac_extractor.jar <javac_arguments>
java -jar /opt/kythe/extractors/javac_extractor.jar \
  -cp third_party/guava/*.jar kythe/java/com/google/devtools/kythe/common/*.java

# Extract a C++ compilation
# /opt/kythe/extractors/cxx_extractor <arguments>
/opt/kythe/extractors/cxx_extractor kythe/cxx/common/net_client.cc \
  -I. -Ibazel-genfiles -Ithird_party/googlelog/src -Ithird_party/rapidjson/include -Ithird_party/proto/src
{% endhighlight %}

## Extracting Compilations using Bazel

Kythe uses Bazel to build itself and has implemented Bazel
[action_listener](http://bazel.io/docs/build-encyclopedia.html#action_listener)s
that use Kythe's Java and C++ extractors.  This effectively allows Bazel to
extract each compilation as it is run during the build.

Add the flag
`--experimental_action_listener=//kythe/java/com/google/devtools/kythe/extractors/java/bazel:extract_kindex`
to make Bazel extract Java compilations and
`--experimental_action_listener=//kythe/cxx/extractor:extract_kindex` to do the
same for C++.

{% highlight bash %}
# Extract all Java/C++ compilations in Kythe
bazel test \
  --experimental_action_listener=//kythe/java/com/google/devtools/kythe/extractors/java/bazel:extract_kindex \
  --experimental_action_listener=//kythe/cxx/extractor:extract_kindex \
  //...

# Find the extracted .kindex files
find -L bazel-out -name '*.kindex'
{% endhighlight %}

The provided utility script,
[https://github.com/google/kythe/blob/master/kythe/extractors/bazel/extract.sh]({{site.data.development.source_browser}}/kythe/extractors/bazel/extract.sh),
does a full extraction using Bazel and then moves the compilations into the
directory structure used by the
[google/kythe]({{site.data.development.source_browser}}/kythe/release/kythe.sh)
Docker image.

## Indexing Compilations

All Kythe indexers analyze compilations emitted from
[extractors](#extracting-compilations) as either a
[.kindex file]({{site.baseurl}}/docs/kythe-index-pack.html#_kindex_format) or an
[index pack]({{site.baseurl}}/docs/kythe-index-pack.html#_index_pack_format).
The indexers will then emit a
[delimited stream]({{site.data.development.source_browser}}/kythe/go/platform/delimited/delimited.go)
of [entry protobufs]({{site.baseurl}}/docs/kythe-storage.html#_entry) that can
then be stored in a [GraphStore]({{site.baseurl}}/docs/kythe-storage.html).

{% highlight bash %}
# Indexing a C++ compilation
# /opt/kythe/indexers/cxx_indexer --ignore_unimplemented <kindex-file> > entries
/opt/kythe/indexers/cxx_indexer --ignore_unimplemented \
  .kythe_compilations/c++/kythe_cxx_indexer_cxx_libIndexerASTHooks.cc.c++.kindex > entries
# /opt/kythe/indexers/cxx_indexer --ignore_unimplemented --index_pack=<root> <unit-hash> > entries
/opt/kythe/indexers/cxx_indexer --ignore_unimplemented \
  --index_pack=.kythe_indexpack f0dcfd6fe90919e957f635ec568a793554905012aea803589cdbec625d72de4d > entries

# Indexing a Java compilation
# java -jar /opt/kythe/indexers/java_indexer.jar <kindex-file> > entries
java -jar /opt/kythe/indexers/java_indexer.jar \
  $PWD/.kythe_compilations/java/kythe_java_com_google_devtools_kythe_analyzers_java_analyzer.java.kindex > entries
# java -jar /opt/kythe/indexers/java_indexer.jar --index_path=<root> <unit-hash> > entries
java -jar /opt/kythe/indexers/java_indexer.jar \
  --index_pack=$PWD/.kythe_indexpack b3759d74b6ee8ba97312cf8b1b47c4263504a56ca9ab63e8f3af98298ccf9fd6 > entries
# NOTE: https://kythe.io/phabricator/T40 -- the Java indexer should not be run in
#       the KYTHE_ROOT_DIRECTORY used during extraction

# View indexer's output entry stream as JSON
/opt/kythe/tools/entrystream --write_json < entries

# Write entry stream into a GraphStore
/opt/kythe/tools/write_entries --graphstore leveldb:/tmp/gs < entries
{% endhighlight %}

## Indexing the Kythe Repository

{% highlight bash %}
mkdir .kythe_{graphstore,compilations}
# .kythe_serving is the output directory for the resulting Kythe serving tables
# .kythe_graphstore is the output directory for the resulting Kythe GraphStore
# .kythe_compilations will contain the intermediary .kindex file for each
#   indexed compilation

# Produce the .kindex files for each compilation in the Kythe repo
./kythe/extractors/bazel/extract.sh "$PWD" .kythe_compilations

# Index the compilations, producing a GraphStore containing a Kythe index
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
bazel run //kythe/go/storage/tools:triples -- --graphstore /path/to/graphstore | \
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

{% highlight bash %}
/opt/kythe/tools/http_server \
  --public_resources /opt/kythe/web/ui \
  --listen localhost:8080 \
  --serving_table .kythe_serving
{% endhighlight %}
