# Release Notes

## Upcoming version

Notable changes:
 - `kythe` tool: merge `--serving_table` flag into `--api` flag

Notable fixes:
 - Allow empty requests in `http_server`'s `/corpusRoots` handler

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
