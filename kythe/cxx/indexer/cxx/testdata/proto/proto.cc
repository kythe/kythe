#include "kythe/javatests/com/google/devtools/kythe/analyzers/java/testdata/pkg/proto/testdata.pb.h"


void fn() {
  using ::pkg::proto::Message;

  //- @Message ref CxxMessage
  Message msg;
  //- @set_string_field ref CxxSetStringField
  msg.set_string_field("value");
  //- @string_field ref CxxGetStringField
  msg.string_field();
  //- @set_int32_field ref CxxSetInt32Field
  msg.set_int32_field(43);
  //- @int32_field ref CxxGetInt32Field
  msg.int32_field();

  //- @NestedMessage ref CxxNestedMessage
  Message::NestedMessage nested;
  //- @set_nested_string ref CxxSetNestedStringField
  nested.set_nested_string("value");
  //- @nested_string ref CxxGetNestedStringField
  nested.nested_string();
  //- @set_nested_bool ref CxxSetNestedBoolField
  nested.set_nested_bool(true);
  //- @nested_bool ref CxxGetNestedBoolField
  nested.nested_bool();


  //- @mutable_nested_message ref CxxSetNestedMessageField
  *msg.mutable_nested_message() = nested;
  //- @nested_message ref CxxGetNestedMessageField
  msg.nested_message();

  //- @clear_oneof_field ref CxxClearOneofField
  msg.clear_oneof_field();
  //- @oneof_field_case ref CxxOneofFieldCase
  msg.oneof_field_case();
  //- @set_oneof_string ref CxxSetOneofString
  msg.set_oneof_string("hello");
}
//- Message generates CxxMessage
//- StringField generates CxxSetStringField
//- StringField generates CxxGetStringField
//- Int32Field generates CxxSetInt32Field
//- Int32Field generates CxxGetInt32Field
//- NestedMessageField generates CxxSetNestedMessageField
//- NestedMessageField generates CxxGetNestedMessageField
//- NestedMessage generates CxxNestedMessage
//- NestedString generates CxxSetNestedStringField
//- NestedString generates CxxGetNestedStringField
//- NestedBool generates CxxSetNestedBoolField
//- NestedBool generates CxxGetNestedBoolField
//- OneofField generates CxxOneofFieldCase
//- OneofField generates CxxClearOneofField
//- OneofString generates CxxSetOneofString
