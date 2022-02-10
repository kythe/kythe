#include "kythe/testdata/indexers/proto/testdata.pb.h"


void fn() {
  using ::pkg::proto::Message;

  //- @Message ref CxxMessage
  Message msg;
  //- @"msg.set_string_field(\"value\")" ref/writes StringField
  msg.set_string_field("value");
  //- @string_field ref CxxGetStringField
  msg.string_field();
  //- @"msg.release_string_field()" ref/writes StringField
  msg.release_string_field();
  //- @"msg.set_allocated_string_field(nullptr)" ref/writes StringField
  msg.set_allocated_string_field(nullptr);
  //- @"msg.set_int32_field(43)" ref/writes Int32Field
  msg.set_int32_field(43);
  //- @int32_field ref CxxGetInt32Field
  msg.int32_field();

  //- @NestedMessage ref CxxNestedMessage
  Message::NestedMessage nested;
  //- @"nested.set_nested_string(\"value\")" ref/writes NestedString
  nested.set_nested_string("value");
  //- @nested_string ref CxxGetNestedStringField
  nested.nested_string();
  //- @"nested.set_nested_bool(true)" ref/writes NestedBool
  nested.set_nested_bool(true);
  //- @nested_bool ref CxxGetNestedBoolField
  nested.nested_bool();


  //- @"msg.mutable_nested_message()" ref/writes NestedMessageField
  *msg.mutable_nested_message() = nested;
  //- @nested_message ref CxxGetNestedMessageField
  msg.nested_message();

  //- @"msg.clear_oneof_field()" ref/writes OneofField
  msg.clear_oneof_field();
  //- @oneof_field_case ref CxxOneofFieldCase
  msg.oneof_field_case();
  //- @"msg.set_oneof_string(\"hello\")" ref/writes OneofString
  msg.set_oneof_string("hello");

  //- @"msg.add_repeated_int32_field(4)" ref/writes RepeatedInt32Field
  msg.add_repeated_int32_field(4);
}
