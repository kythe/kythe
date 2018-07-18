# Release Notes

## [Unreleased]
Nothing to report yet.

## [v0.0.28] - 2018-07-18

### Added
 - `write_tables`: `--experimental_beam_pipeline` runs the post-processor as an
   [Apache Beam] pipeline
 - Go: support extracting/analyzing packages with `vendor/` dirs
 - `go_indexer`: add `--continue` flag to log errors without halting analysis
 - `viewindex`: add --file flag to print a single file's contents
 - `entrystream`: support for reading/writing [Riegeli] record files
 - First release of the [ExploreService] APIs

### Deprecated
 - `entrystream`: `--read_json`/`--write_json` are marked to be replaced by
   `--read_format=json`/`--write_format=json`

### Fixed
 - Java indexer: search for ClassSymbol in all Java 9 modules
 - Java/JVM indexers: obtain JVM (a.k.a. "external") names in a principled way

[Apache Beam]: https://beam.apache.org/
[ExploreService]: https://github.com/google/kythe/blob/master/kythe/proto/explore.proto
[Riegeli]: https://github.com/google/riegeli

## [v0.0.27] - 2018-07-01

Due to the period of time between this release and v0.0.26, many relevant
changes and fixes may not appear in the following list.  For a complete list of
changes, please check the commit logs:
https://github.com/google/kythe/compare/v0.0.26...v0.0.27

### Added
 - First release of Go indexer and Go extractors.
 - Objective C is now supported in the C++ indexer.
 - identifier.proto: adds a new `IdentifierService` API.
 - Runtime plugins can be added to the Java indexer.

### Changed
 - Remove gRPC server and client code.
 - Schema:
   - Anchors no longer have `childof` edges to their parent `file`.
   - Anchor nodes must share their `path`, `root`, and `corpus` `VName`
     components with their parent `file`.
  - Format strings have been replaced by the `MarkedSource` protobuf message.
 - C++ analysis:
   - Included files are referenced by the `ref/file` edge.
   - `code` facts (containing `MarkedSource` protos) are emitted.
   - Better support for C++11, 14, and 17.
   - Limited CUDA support.
   - Improvements to indexing and rendering of documentation.
   - Added a plugin for indexing proto fields in certain string literals.
   - Type ranges for builtin structural types are no longer destructured (`T*`
     is now `[T]*`, not `[[T]*]`).
   - Decoration of builtin types can be controlled with
     `--emit_anchors_on_builtins`.
   - Namespaces are never `define`d, only `ref`d.
   - Old-style `name` nodes removed.
   - The extractor now captures more build state, enabling support for
     `__has_include`
   - `anchor`s involved in `completes` edges now contain their targets mixed
     with their signatures, making each completion relationship unique.
   - Support for indexing uses of `make_unique` et al as direct references to
     the relevant constructor.
   - Template instantiations can be aliased together with
     `--experimental_alias_template_instantiations`, which significantly
     decreases output size at the expense of lower fidelity.
 - Java analysis:
   - `name` nodes are now defined as JVM binary names.
   - `diagnostic` nodes are emitted on errors.
   - `code` facts (`MarkedSource` protos) are emitted.
   - Add callgraph edges for constructors.
   - Non-member classes are now `childof` their enclosing method.
   - Local variables are now `childof` their enclosing method.
   - Blame calls in static initializers on the enclosing class.
   - Emit references for all matching members of a static import.
   - Reference `abs` nodes for generic classes instead of their `record` nodes.
   - Emit data for annotation arguments.
   - Emit package relations from `package-info.java` files.
 - Protocol Buffers:
   - analysis.proto: add revision and build ID to AnalysisRequest.
   - analysis.proto: add AnalysisResult summary to AnalysisOutput.
   - analysis.proto: remove `revision` from CompilationUnit.
   - graph.proto: new location of the GraphService API
   - xref.proto: remove `DecorationsReply.Reference.source_ticket`.
   - xref.proto: add `Diagnostic` messages to `DecorationsReply`.
   - xref.proto: replace `Location.Point` pairs with `common.Span`.
   - xref.proto: by default, elide snippets from xrefs/decor replies.
   - xref.proto: replace `Printable` formats with `MarkedSource`.
   - xref.proto: allow filtering related nodes in the xrefs reply.
   - xref.proto: optionally return documentation children.
   - xref.proto: return target definitions with overrides.

## [v0.0.26] - 2016-11-11

### Changed
 - Nodes and Edges API calls have been moved from XRefService to GraphService.

## [v0.0.25] - 2016-10-28

### Changed
 - Replace google.golang.org/cloud dependencies with cloud.google.com/go
 - Update required version of Go from 1.6 to 1.7

## [v0.0.24] - 2016-08-16

### Fixed
 - write_tables now tolerates nodes with no facts. Previously it could sometimes crash if this occurred.

## [v0.0.23] - 2016-07-28

### Changed
 - CrossReferences API: hide signature generation behind feature flag
 - Java indexer: emit `ref/imports` anchors for imported symbols

### Added
 - Java indexer: emit basic `format` facts

## [v0.0.22] - 2016-07-20

### Changed
 - Schema: `callable` nodes and `callableas` edges have been removed.
 - `xrefs.CrossReferences`: change Anchors in the reply to RelatedAnchors
 - Removed search API

## [v0.0.21] - 2016-05-12

### Changed
 - xrefs service: replace most repeated fields with maps
 - xrefs service: add `ordinal` field to each EdgeSet edge
 - `xrefs.CrossReferences`: group declarations/definitions for incomplete nodes
 - C++ indexer: `--flush_after_each_entry` now defaults to `true`

### Added
 - `xrefs.Decorations`: add experimental `target_definitions` switch
 - kythe tool: add `--graphviz` output flag to `edges` subcommand
 - kythe tool: add `--target_definitions` switch to `decor` subcommand

### Fixed
  `write_tables`: correctly handle nodes with missing facts
 - Javac extractor: add processors registered in META-INF/services
 - javac-wrapper.sh: prepend bootclasspath jar to use packaged javac tools

## [v0.0.20] - 2016-03-03

### Fixed
 - Java indexer: reduce redundant AST traversals causing large slowdowns

## [v0.0.19] - 2016-02-26

### Changed
 - C++ extractor: `KYTHE_ROOT_DIRECTORY` no longer changes the working
   directory during extraction, but does still change the root for path
   normalization.
 - `http_server`: ensure the given `--serving_table` exists (do not create, if missing)
 - Java indexer: fixes/tests for interfaces, which now have `extends` edges
 - `kythe` tool: display subkinds for related nodes in xrefs subcommand

### Added
 - `entrystream`: add `--unique` flag
 - `write_tables`: add `--entries` flag

## [v0.0.18] - 2016-02-11

### Changed
 - C++ indexer: `--ignore_unimplemented` now defaults to `true`
 - Java indexer: emit single anchor for package in qualified identifiers

### Added
 - Java indexer: add callgraph edges
 - Java indexer: add Java 8 member reference support

## [v0.0.17] - 2015-12-16

### Added
 - `write_tables`: produce serving data for xrefs.CrossReferences method
 - `write_tables`: add flags to tweak performance
     - `--compress_shards`: determines whether intermediate data written to disk
       should be compressed
     - `--max_shard_size`: maximum number of elements (edges, decoration
       fragments, etc.) to keep in-memory before flushing an intermediary data
       shard to disk
     - `--shard_io_buffer`: size of the reading/writing buffers for the
       intermediary data shards

## [v0.0.16] - 2015-12-08

### Changed
 - Denormalize the serving table format
 - xrefs.Decorations: only return Reference targets in DecorationsReply.Nodes
 - Use proto3 JSON mapping for web requests: https://developers.google.com/protocol-buffers/docs/proto3#json
 - Java indexer: report error when indexing from compilation's source root
 - Consistently use corpus root relative paths in filetree API
 - Java, C++ indexer: ensure file node VNames to be schema compliant
 - Schema: File nodes should no longer have the `language` VName field set

### Added
 - Java indexer: emit (possibly multi-line) snippets over entire surrounding statement
 - Java indexer: emit class node for static imports

### Fixed
 - Java extractor: correctly parse @file arguments using javac CommandLine parser
 - Java extractor: correctly parse/load -processor classes
 - xrefs.Edges: correctly return empty page_token on last page (when filtering by edge kinds)

## [v0.0.15] - 2015-10-13

### Changed
 - Java 8 is required for the Java extractor/indexer

### Fixed
 - `write_tables`: don't crash when given a node without any edges
 - Java extractor: ensure output directory exists before writing kindex

## [v0.0.14] - 2015-10-08

### Fixed
 - Bazel Java extractor: filter out Bazel-specific flags
 - Java extractor/indexer: filter all unsupported options before yielding to the compiler

## [v0.0.13] - 2015-09-17

### Added
 - Java indexer: add `ref/doc` anchors for simple class references in JavaDoc
 - Java indexer: emit JavaDoc comments more consistently; emit enum documentation

## [v0.0.12] - 2015-09-10

### Changed
 - C++ indexer: rename `/kythe/edge/defines` to `/kythe/edge/defines/binding`
 - Java extractor: change failure to warning on detection of non-java sources
 - Java indexer: `defines` anchors span an entire class/method/var definition (instead of
                 just their identifier; see below for `defines/binding` anchors)
 - Add public protocol buffer API/message definitions

### Added
 - Java indexer: `ref` anchors span import packages
 - Java indexer: `defines/binding` anchors span a definition's identifier (identical
                  behavior to previous `defines` anchors)
 - `http_server`: add `--http_allow_origin` flag that adds the `Access-Control-Allow-Origin` header to each HTTP response

## [v0.0.11] - 2015-09-01

### Added
 - Java indexer: name node support for array types, builtins, files, and generics

### Fixed
 - Java indexer: stop an exception from being thrown when a line contains multiple comments

## [v0.0.10] - 2015-08-31

### Added
 - `http_server`: support TLS HTTP2 server interface
 - Java indexer: broader `name` node coverage
 - Java indexer: add anchors for method/field/class definition comments
 - `write_table`: add `--max_edge_page_size` flag to control the sizes of each
                  PagedEdgeSet and EdgePage written to the output table

### Fixed
 - `entrystream`: prevent panic when given `--entrysets` flag

## [v0.0.9] - 2015-08-25

### Changed
 - xrefs.Decorations: nodes will not be populated unless given a fact filter
 - xrefs.Decorations: each reference has its associated anchor start/end byte offsets
 - Schema: loosened restrictions on VNames to permit hashing

### Added
 - dedup_stream: add `--cache_size` flag to limit memory usage
 - C++ indexer: hash VNames whenever permitted to reduce output size

### Fixed
 - write_tables: avoid deadlock in case of errors

## [v0.0.8] - 2015-07-27

### Added
 - Java extractor: add JavaDetails to each CompilationUnit
 - Release the indexer verifier tool (see http://www.kythe.io/docs/kythe-verifier.html)

### Fixed
 - write_tables: ensure that all edges are scanned for FileDecorations
 - kythe refs command: normalize locations within dirty buffer, if given one

## [v0.0.7] - 2015-07-16

### Changed
 - Dependencies: updated minimum LLVM revision. Run tools/modules/update.sh.
 - C++ indexer: index definitions and references to member variables.
 - kwazthis: replace `--ignore_local_repo` behavior with `--local_repo=NONE`

### Added
 - kwazthis: if found, automatically send local file as `--dirty_buffer`
 - kwazthis: return `/kythe/edge/typed` target ticket for each node

## [v0.0.6] - 2015-07-09

### Added
 - kwazthis: allow `--line` and `--column` info in place of a byte `--offset`
 - kwazthis: the `--api` flag can now handle a local path to a serving table

### Fixed
 - Java indexer: don't generate anchors for implicit constructors

## [v0.0.5] - 2015-07-01

### Added
 - Bazel `extra_action` extractors for C++ and Java
 - Implementation of DecorationsRequest.dirty_buffer in xrefs serving table

## [v0.0.4] - 2015-07-24

### Changed
 - `kythe` tool: merge `--serving_table` flag into `--api` flag

### Fixed
 - Allow empty requests in `http_server`'s `/corpusRoots` handler
 - Java extractor: correctly handle symlinks in KYTHE_ROOT_DIRECTORY

## [v0.0.3] - 2015-07-16

### Changed
 - Go binaries no longer require shared libraries for libsnappy or libleveldb
 - kythe tool: `--log_requests` global flag
 - Java indexer: `--print_statistics` flag

## [v0.0.2] - 2015-06-05

### Changed
 - optimized binaries
 - more useful CLI `--help` messages
 - remove sqlite3 GraphStore support
 - kwazthis: list known definition locations for each node
 - Java indexer: emit actual nodes for JDK classes

## [v0.0.1] - 2015-05-20

Initial release

[Unreleased] https://github.com/google/kythe/compare/v0.0.28...HEAD
[v0.0.28]: https://github.com/google/kythe/compare/v0.0.27...v0.0.28
[v0.0.27]: https://github.com/google/kythe/compare/v0.0.26...v0.0.27
[v0.0.26]: https://github.com/google/kythe/compare/v0.0.25...v0.0.26
[v0.0.25]: https://github.com/google/kythe/compare/v0.0.24...v0.0.25
[v0.0.24]: https://github.com/google/kythe/compare/v0.0.23...v0.0.24
[v0.0.23]: https://github.com/google/kythe/compare/v0.0.22...v0.0.23
[v0.0.22]: https://github.com/google/kythe/compare/v0.0.21...v0.0.22
[v0.0.21]: https://github.com/google/kythe/compare/v0.0.20...v0.0.21
[v0.0.20]: https://github.com/google/kythe/compare/v0.0.19...v0.0.20
[v0.0.19]: https://github.com/google/kythe/compare/v0.0.18...v0.0.19
[v0.0.18]: https://github.com/google/kythe/compare/v0.0.17...v0.0.18
[v0.0.17]: https://github.com/google/kythe/compare/v0.0.16...v0.0.17
[v0.0.16]: https://github.com/google/kythe/compare/v0.0.15...v0.0.16
[v0.0.15]: https://github.com/google/kythe/compare/v0.0.14...v0.0.15
[v0.0.14]: https://github.com/google/kythe/compare/v0.0.13...v0.0.14
[v0.0.13]: https://github.com/google/kythe/compare/v0.0.12...v0.0.13
[v0.0.12]: https://github.com/google/kythe/compare/v0.0.11...v0.0.12
[v0.0.11]: https://github.com/google/kythe/compare/v0.0.10...v0.0.11
[v0.0.10]: https://github.com/google/kythe/compare/v0.0.9...v0.0.10
[v0.0.9]: https://github.com/google/kythe/compare/v0.0.8...v0.0.9
[v0.0.8]: https://github.com/google/kythe/compare/v0.0.7...v0.0.8
[v0.0.7]: https://github.com/google/kythe/compare/v0.0.6...v0.0.7
[v0.0.6]: https://github.com/google/kythe/compare/v0.0.5...v0.0.6
[v0.0.5]: https://github.com/google/kythe/compare/v0.0.4...v0.0.5
[v0.0.4]: https://github.com/google/kythe/compare/v0.0.3...v0.0.4
[v0.0.3]: https://github.com/google/kythe/compare/v0.0.2...v0.0.3
[v0.0.2]: https://github.com/google/kythe/compare/v0.0.1...v0.0.2
[v0.0.1]: https://github.com/google/kythe/compare/d3b7a50...v0.0.1
