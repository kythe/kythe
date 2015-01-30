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

## Extracting Compilations

{% highlight bash %}
# Environment variables common to Kythe extractors
export KYTHE_ROOT_DIRECTORY="$PWD"                        # Root of source code corpus
export KYTHE_OUTPUT_DIRECTORY="/tmp/kythe.compilations/"  # Output directory
export KYTHE_VNAMES="$PWD/kythe/data/vnames.json"         # Optional: VNames configuration

mkdir -p "$KYTHE_OUTPUT_DIRECTORY"

# Extract a Java compilation
JAVAC_EXTRACTOR=//kythe/java/com/google/devtools/kythe/extractors/java/standalone:javac_extractor
# ./campfire run "$JAVAC_EXTRACTOR" <javac_arguments>
./campfire run "$JAVAC_EXTRACTOR" \
  -cp "campfire-out/bin/kythe/proto/storage_proto.jar:third_party/protobuf/protobuf-java-2.5.0.jar:third_party/guava/guava-16.0.1.jar" \
  kythe/java/com/google/devtools/kythe/util/KytheURI.java

# Extract a C++ compilation
CXX_EXTRACTOR=//kythe/cxx/extractor:cxx_extractor
# ./campfire run "$CXX_EXTRACTOR" <arguments>
./campfire run "$CXX_EXTRACTOR" \
  kythe/cxx/verifier/assertions.tab.cc \
  -Wno-deprecated-register -I campfire-out/gen/kythe/proto/storage_proto/cxx -I third_party/gflags/include -I third_party/glog/include -I third_party/protobuf/include -I. -std=c++11 -pthread -fno-rtti -Wall -Werror -Wno-unused-variable -DCAMPFIRE_CONFIGURATION=amd64
{% endhighlight %}

## Extracting Compilations using Campfire

Kythe's build system, [campfire]({{site.baseurl}}/docs/campfire.html), has
built-in support for Kythe's extractors and can run them as part of its build
process using `./campfire extract`, greatly simplifying the process.  All
resulting compilations will be the `campfire-out/gen` directory: `find
campfire-out/gen -name "*.kindex"`.

The provided utility script,
[kythe/extractors/campfire/extract.sh]({{site.data.development.source_browser}}/kythe/extractors/campfire/extract.sh),
does a full extraction using campfire and then moves the compilations into the
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
CXX_INDEXER='//kythe/cxx/indexer/cxx:indexer'
# ./campfire run $CXX_INDEXER --ignore_unimplemented <kindex-file> > entries
./campfire run $CXX_INDEXER --ignore_unimplemented \
  .kythe_compilations/c++/kythe_cxx_indexer_cxx_libIndexerASTHooks.cc.c++.kindex > entries
# ./campfire run $CXX_INDEXER --ignore_unimplemented --index_pack=<root> <unit-hash> > entries
./campfire run $CXX_INDEXER --ignore_unimplemented \
  --index_pack=.kythe_indexpack f0dcfd6fe90919e957f635ec568a793554905012aea803589cdbec625d72de4d > entries

# Indexing a Java compilation
JAVA_INDEXER='//kythe/java/com/google/devtools/kythe/analyzers/java:indexer'
# ./campfire run $JAVA_INDEXER <kindex-file> > entries
./campfire run --run_cwd=/tmp $JAVA_INDEXER \
  $PWD/.kythe_compilations/java/kythe_java_com_google_devtools_kythe_analyzers_java_analyzer.java.kindex > entries
# ./campfire run $JAVA_INDEXER --index_path=<root> <unit-hash> > entries
./campfire run --run_cwd=/tmp $JAVA_INDEXER \
  --index_pack=$PWD/.kythe_indexpack b3759d74b6ee8ba97312cf8b1b47c4263504a56ca9ab63e8f3af98298ccf9fd6 > entries
# NOTE: --run_cwd=/tmp is meant to ensure that the indexer does not succumb to
#       T40 for Kythe compilations

# View indexer's output entry stream as JSON
./campfire run //kythe/go/platform/tools:entrystream --write_json < entries

# Write entry stream into a GraphStore
./campfire run //kythe/go/storage/tools:write_entries --graphstore leveldb:/tmp/gs < entries
{% endhighlight %}

## Indexing the Kythe Repository

{% highlight bash %}
./campfire package //kythe/release # build the Kythe Docker container

mkdir .kythe_{graphstore,compilations}
# .kythe_graphstore is the output directory for the resulting Kythe GraphStore
# .kythe_compilations will contain the intermediary .kindex file for each
#   indexed compilation

# Produce the .kindex files for each compilation in the Kythe repo
./kythe/extractors/campfire/extract.sh "$PWD" .kythe_compilations

# Index the compilations, producing a GraphStore containing a Kythe index
docker run --rm \
  -v "${PWD}:/repo" \
  -v "${PWD}/.kythe_compilations:/compilations" \
  -v "${PWD}/.kythe_graphstore:/graphstore" \
  google/kythe --index
{% endhighlight %}

## Visualizing Cross-References

As part of Kythe's first release, a sample UI has been made to show Kythe's
basic cross-reference capabilities.

{% highlight bash %}
pushd kythe/web/ui
lein cljsbuild once prod # Build the necessary client-side code
popd

./campfire run //kythe/go/serving/tools:http_server \
  --public_resources kythe/web/ui/resources/public \
  --listen localhost:8080 --graphstore .kythe_graphstore
{% endhighlight %}
