#pragma once

namespace kythe_indexer_test {

// If pragma once breaks, this struct will be defined multiple times
// and trigger an error, failing the test.
struct Empty{};

}  // namespace kythe_indexer_test
