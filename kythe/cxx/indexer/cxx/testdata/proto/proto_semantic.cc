#include "kythe/testdata/indexers/proto/testdata.pb.h"


void fn() {
  using ::pkg::proto::Message;

  //- @Message ref CxxMessage
  Message msg;
  //- @set_string_field ref/writes StringField
  msg.set_string_field("value");
  //- @string_field ref CxxGetStringField
  msg.string_field();
  //- // @set_int32_field ref/writes Int32Field
  msg.set_int32_field(43);
  //- @int32_field ref CxxGetInt32Field
  msg.int32_field();

  //- @NestedMessage ref CxxNestedMessage
  Message::NestedMessage nested;
  //- @set_nested_string ref/writes NestedString
  nested.set_nested_string("value");
  //- @nested_string ref CxxGetNestedStringField
  nested.nested_string();
  //- // @set_nested_bool ref/writes NestedBool
  nested.set_nested_bool(true);
  //- @nested_bool ref CxxGetNestedBoolField
  nested.nested_bool();


  //- @mutable_nested_message ref/writes NestedMessageField
  *msg.mutable_nested_message() = nested;
  //- @mutable_nested_message ref NestedMessageField
  //- !{ @mutable_nested_message ref/writes NestedMessageField }
  msg.mutable_nested_message();
  //- @nested_message ref CxxGetNestedMessageField
  msg.nested_message();

  //- // @clear_oneof_field ref/writes OneofField
  msg.clear_oneof_field();
  //- @oneof_field_case ref CxxOneofFieldCase
  msg.oneof_field_case();
  //- @set_oneof_string ref/writes OneofString
  msg.set_oneof_string("hello");

  //- // @add_repeated_int32_field ref/writes RepeatedInt32Field
  msg.add_repeated_int32_field(4);
}
