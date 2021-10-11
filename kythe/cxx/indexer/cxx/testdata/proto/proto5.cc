#include "kythe/testdata/indexers/proto/gen/testdata5.gen.pb.h"

void fn() {

  using ::pkg::proto5::Message;

  //- @Message ref CxxMessage
  Message msg;
  //- @set_string_field ref CxxSetStringFieldTapp
  msg.set_string_field("value");
  //- @string_field ref CxxGetStringField
  msg.string_field();
}
//- Message generates CxxMessage
//- CxxSetStringFieldTapp param.0 CxxSetStringFieldAbs
//- CxxSetStringFieldAbsBindingAnchor defines/binding CxxSetStringFieldAbs
//- CxxSetStringFieldAbsBindingAnchor completes/uniquely SetStringFieldAbs
//- StringField generates SetStringFieldAbs
//- StringField generates CxxGetStringField
