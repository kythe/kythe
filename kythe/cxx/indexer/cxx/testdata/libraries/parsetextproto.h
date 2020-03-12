#ifndef KYTHE_CXX_INDEXER_CXX_TESTDATA_LIBRARIES_PROTO_PARSETEXTPROTO_H_
#define KYTHE_CXX_INDEXER_CXX_TESTDATA_LIBRARIES_PROTO_PARSETEXTPROTO_H_

#define PARSE_TEXT_PROTO(asciipb) \
  ::proto2::contrib::parse_proto::ParseTextProtoOrDie(asciipb, {}, {})

#define PARSE_PARTIAL_TEXT_PROTO(asciipb)              \
  ::proto2::contrib::parse_proto::ParseTextProtoOrDie( \
      asciipb, ::proto2::contrib::parse_proto::AllowPartial(), {})

// The following declarations mimick the code structure of ParseProtoHelper.
namespace absl {
struct string_view {
  string_view(const char* str) {}  // NOLINT
};

struct SourceLocation {};
}  // namespace absl

namespace proto2 {
class Message;
namespace contrib {
namespace parse_proto {
namespace internal {
struct ParseProtoHelper;
}  // namespace internal

struct ParserConfig {};
ParserConfig AllowPartial();

internal::ParseProtoHelper ParseTextProtoOrDie(absl::string_view asciipb,
                                               ParserConfig config,
                                               absl::SourceLocation loc = {});
internal::ParseProtoHelper ParseTextProtoOrDie(absl::string_view asciipb,
                                               absl::SourceLocation loc = {});

namespace internal {
struct ParseProtoHelper {
  template <typename T>
  operator T() const;
};

}  // namespace internal
}  // namespace parse_proto
}  // namespace contrib
}  // namespace proto2

#endif  // KYTHE_CXX_INDEXER_CXX_TESTDATA_LIBRARIES_PROTO_PARSETEXTPROTO_H_
