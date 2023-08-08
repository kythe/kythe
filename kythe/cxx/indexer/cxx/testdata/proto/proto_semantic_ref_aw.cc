#include "kythe/testdata/indexers/proto/testdata.pb.h"


void fn() {
  using ::pkg::proto::Message;

  //- @Message ref CxxMessage
  Message msg;
  //- @set_string_field ref/writes StringField
  //- @"msg.set_string_field(\"value\")" ref/call CxxSetStringField
  //- @set_string_field ref CxxSetStringField
  //- !{ @set_string_field ref StringField }
  msg.set_string_field("value");
  //- @string_field ref CxxGetStringField
  msg.string_field();
  //- StringField generates CxxGetStringField

  Message::NestedMessage nested;
  //- @mutable_nested_message ref/writes NestedMessageField
  //- @"msg.mutable_nested_message()" ref/call CxxMutableNestedMessage
  //- @mutable_nested_message ref CxxMutableNestedMessage
  //- !{ @mutable_nested_message ref NestedMessageField }
  *msg.mutable_nested_message() = nested;
  //- NestedMessageField generates CxxMutableNestedMessage

  //- @mutable_nested_message ref/writes NestedMessageField
  //- !{ @mutable_nested_message ref NestedMessageField }
  *msg.mutable_nested_message();
}
