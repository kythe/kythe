# Release Notes

## v0.0.9

Notable changes:
 - xrefs.Decorations: nodes will not be populated unless given a fact filter
 - xrefs.Decorations: each reference has its associated anchor start/end byte offsets
 - Schema: loosened restrictions on VNames to permit hashing

Notable additions:
 - dedup_stream: add `--cache_size` flag to limit memory usage
 - C++ indexer: hash VNames whenever permitted to reduce output size

Notable fixes:
 - write_tables: avoid deadlock in case of errors

## v0.0.8

Notable additions:
 - Java extractor: add JavaDetails to each CompilationUnit
 - Release the indexer verifier tool (see http://www.kythe.io/docs/kythe-verifier.html)

Notable fixes:
 - write_tables: ensure that all edges are scanned for FileDecorations
 - kythe refs command: normalize locations within dirty buffer, if given one

## v0.0.7

Notable changes:
 - Dependencies: updated minimum LLVM revision. Run tools/modules/update.sh.
 - C++ indexer: index definitions and references to member variables.
 - kwazthis: replace `--ignore_local_repo` behavior with `--local_repo=NONE`

Notable additions:
 - kwazthis: if found, automatically send local file as `--dirty_buffer`
 - kwazthis: return `/kythe/edge/typed` target ticket for each node

## v0.0.6

Notable additions:
 - kwazthis: allow `--line` and `--column` info in place of a byte `--offset`
 - kwazthis: the `--api` flag can now handle a local path to a serving table

Notable fixes:
 - Java indexer: don't generate anchors for implicit constructors

## v0.0.5

Notable additions:
 - Bazel `extra_action` extractors for C++ and Java
 - Implementation of DecorationsRequest.dirty_buffer in xrefs serving table

## v0.0.4

Notable changes:
 - `kythe` tool: merge `--serving_table` flag into `--api` flag

Notable fixes:
 - Allow empty requests in `http_server`'s `/corpusRoots` handler
 - Java extractor: correctly handle symlinks in KYTHE_ROOT_DIRECTORY

## v0.0.3

Notable changes:
 - Go binaries no longer require shared libraries for libsnappy or libleveldb
 - kythe tool: `--log_requests` global flag
 - Java indexer: `--print_statistics` flag

## v0.0.2

Notable changes:
 - optimized binaries
 - more useful CLI `--help` messages
 - remove sqlite3 GraphStore support
 - kwazthis: list known definition locations for each node
 - Java indexer: emit actual nodes for JDK classes

## v0.0.1

Initial release
