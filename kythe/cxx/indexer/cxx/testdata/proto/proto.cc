#include "kythe/testdata/indexers/proto/testdata.pb.h"


void fn() {
  using ::pkg::proto::Message;

  //- @Message ref CxxMessage
  Message msg;
  //- @set_string_field ref CxxSetStringFieldTapp
  msg.set_string_field("value");
  //- @string_field ref CxxGetStringField
  msg.string_field();
  //- @set_int32_field ref CxxSetInt32Field
  msg.set_int32_field(43);
  //- @int32_field ref CxxGetInt32Field
  msg.int32_field();

  //- @NestedMessage ref CxxNestedMessage
  Message::NestedMessage nested;
  //- @set_nested_string ref CxxSetNestedStringFieldTapp
  nested.set_nested_string("value");
  //- @nested_string ref CxxGetNestedStringField
  nested.nested_string();
  //- @set_nested_bool ref CxxSetNestedBoolField
  nested.set_nested_bool(true);
  //- @nested_bool ref CxxGetNestedBoolField
  nested.nested_bool();


  //- @"msg.mutable_nested_message()" ref/call CxxSetNestedMessageField
  //- @mutable_nested_message ref/writes NestedMessageField
  *msg.mutable_nested_message() = nested;
  //- @nested_message ref CxxGetNestedMessageField
  msg.nested_message();

  //- @clear_oneof_field ref CxxClearOneofField
  msg.clear_oneof_field();
  //- @oneof_field_case ref CxxOneofFieldCase
  msg.oneof_field_case();
  //- @set_oneof_string ref CxxSetOneofStringTapp
  msg.set_oneof_string("hello");

  //- @repeated_int32_field ref CxxRepeatedInt32Field
  msg.repeated_int32_field(1);
}
//- Message generates CxxMessage
//- CxxSetStringFieldTapp param.0 CxxSetStringFieldAbs
//- _CxxSetStringFieldAbsBindingAnchor defines/binding CxxSetStringFieldAbs
//- SetStringFieldAbs completedby CxxSetStringFieldAbs
//- StringField generates SetStringFieldAbs
//- StringField generates CxxGetStringField
//- Int32Field generates CxxSetInt32Field
//- Int32Field generates CxxGetInt32Field
//- NestedMessageField generates CxxSetNestedMessageField
//- NestedMessageField generates CxxGetNestedMessageField
//- NestedMessage generates CxxNestedMessage
//- CxxSetNestedStringFieldTapp param.0 CxxSetNestedStringFieldAbs
//- _CxxSetNestedStringFieldAbsBindingAnchor defines/binding CxxSetNestedStringFieldAbs
//- SetNestedStringFieldAbs completedby CxxSetNestedStringFieldAbs
//- NestedString generates SetNestedStringFieldAbs
//- NestedString generates CxxGetNestedStringField
//- NestedBool generates CxxSetNestedBoolField
//- NestedBool generates CxxGetNestedBoolField
//- OneofField generates CxxOneofFieldCase
//- OneofField generates CxxClearOneofField
//- CxxSetOneofStringTapp param.0 CxxSetOneofStringAbs
//- _CxxSetOneofStringAbsBindingAnchor defines/binding CxxSetOneofStringAbs
//- SetOneofStringAbs completedby CxxSetOneofStringAbs
//- OneofString generates SetOneofStringAbs
//- RepeatedInt32Field generates CxxRepeatedInt32Field
