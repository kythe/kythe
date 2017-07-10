#ifndef KYTHE_CXX_INDEXER_CXX_TESTDATA_LIBRARIES_PROTO_PARSETEXTPROTO_H_
#define KYTHE_CXX_INDEXER_CXX_TESTDATA_LIBRARIES_PROTO_PARSETEXTPROTO_H_

// The following declarations mimick the code structure of ParseProtoHelper.
struct StringPiece {
  StringPiece(const char* str) {}  // NOLINT
};

namespace proto2 {
class Message;
namespace contrib {
namespace parse_proto {
namespace internal {
struct ParseProtoHelper {
  template <typename T>
  operator T() const;
};
}  // namespace internal

internal::ParseProtoHelper ParseTextProtoOrDieAt(StringPiece asciipb,
                                                 bool allow_partial,
                                                 StringPiece file, int line);

}  // namespace parse_proto
}  // namespace contrib
}  // namespace proto2

#define PARSE_TEXT_PROTO(asciipb) \
    ::proto2::contrib::parse_proto::ParseTextProtoOrDieAt( \
        asciipb, false, __FILE__, __LINE__)

#endif  // KYTHE_CXX_INDEXER_CXX_TESTDATA_LIBRARIES_PROTO_PARSETEXTPROTO_H_
