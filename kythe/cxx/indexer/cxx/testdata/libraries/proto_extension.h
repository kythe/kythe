#ifndef KYTHE_CXX_INDEXER_CXX_TESTDATA_LIBRARIES_PROTO_EXTENSION_H_
#define KYTHE_CXX_INDEXER_CXX_TESTDATA_LIBRARIES_PROTO_EXTENSION_H_

// The following declarations mimick the proto2 Extension API.
namespace proto2 {

namespace bridge {

class MessageSet {};

}  // namespace bridge

namespace internal {

template <typename Type>
class PrimitiveTypeTraits {};
template <typename Type>
class MessageTypeTraits {};

template <typename ExtendeeType, typename TypeTraitsType>
class ExtensionIdentifier {};

}  // namespace internal

}  // namespace proto2

#endif  // KYTHE_CXX_INDEXER_CXX_TESTDATA_LIBRARIES_PROTO_EXTENSION_H_
