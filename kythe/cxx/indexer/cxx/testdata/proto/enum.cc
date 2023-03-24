#include "kythe/testdata/indexers/proto/enum.pb.h"

//- WrapperMessageProto generates WrapperMessageCpp
//- EnumerationTypeProto generates EnumerationTypeCpp
//- EnumeratorProto generates EnumeratorCpp

//- @Wrapper ref WrapperMessageCpp
//- @EnumerationType ref EnumerationAliasCpp
//- EnumerationTypeProto generates EnumerationAliasCpp
::pkg::proto::Wrapper::EnumerationType get_alias();
//- @Wrapper_EnumerationType ref EnumerationTypeCpp
::pkg::proto::Wrapper_EnumerationType get_raw();

void fn() {
  //- @Wrapper ref WrapperMessageCpp
  ::pkg::proto::Wrapper message;
  //- @Wrapper ref WrapperMessageCpp
  //- @ENUMERATOR ref EnumeratorCpp
  message.set_field(::pkg::proto::Wrapper::ENUMERATOR);
}
