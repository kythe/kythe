package pkg.proto;

import static pkg.proto.Testdata.Message;

public class Proto {

  //- @Message ref JavaMessage
  static final Message MSG =
      Message.newBuilder()
          //- @setStringField ref JavaSetStringField
          .setStringField("blah")
          //- @clearStringField ref JavaClearStringField
          .clearStringField()
          //- @setInt32Field ref JavaSetInt32Field
          .setInt32Field(43)
          //- @clearInt32Field ref JavaClearInt32Field
          .clearInt32Field()
          //- @setNestedMessage ref JavaSetNestedMessageField
          .setNestedMessage(
              //- @NestedMessage ref JavaNestedMessage
              Message.NestedMessage.newBuilder()
                  //- @setNestedString ref JavaSetNestedString
                  .setNestedString("nested")
                  // TODO(justbuchanan): re-enable this test once protobuf is
                  // updated to annotate this correctly
                  // //- @clearNestedString ref JavaClearNestedString
                  .clearNestedString()
                  //- @setNestedBool ref JavaSetNestedBool
                  .setNestedBool(true)
                  //- @clearNestedBool ref JavaClearNestedBool
                  .clearNestedBool()
                  .build())
          //- @clearNestedMessage ref JavaClearNestedMessageField
          .clearNestedMessage()
          //- @setOneofString ref JavaSetOneofString
          .setOneofString("hello")
          //- @clearOneofString ref JavaClearOneofString
          .clearOneofString()
          //- @addRepeatedInt32Field ref JavaAddRepeatedInt32Field
          .addRepeatedInt32Field(44)
          //- @clearRepeatedInt32Field ref JavaClearRepeatedInt32Field
          .clearRepeatedInt32Field()
          .build();

  static void getFields() {
    //- @getStringField ref JavaGetStringField
    print(MSG.getStringField());
    //- @getInt32Field ref JavaGetInt32Field
    print(MSG.getInt32Field());

    //- @getNestedMessage ref JavaGetNestedMessageField
    Message.NestedMessage nested = MSG.getNestedMessage();
    //- @getNestedString ref JavaGetNestedString
    print(nested.getNestedString());
    //- @getNestedBool ref JavaGetNestedBool
    print(nested.getNestedBool());
  }

  static void print(Object obj) {
    System.out.println(obj);
  }

  //- Message generates JavaMessage
  //- StringField generates JavaSetStringField
  //- StringField generates JavaGetStringField
  //- StringField generates JavaClearStringField
  //- Int32Field generates JavaSetInt32Field
  //- Int32Field generates JavaGetInt32Field
  //- Int32Field generates JavaClearInt32Field
  //- NestedMessageField generates JavaSetNestedMessageField
  //- NestedMessageField generates JavaGetNestedMessageField


  // TODO(justbuchanan): re-enable this test once protobuf is updated to
  // annotate this correctly
  // //- NestedMessageField generates JavaClearNestedMessageField

  //- NestedMessage generates JavaNestedMessage
  //- NestedString generates JavaSetNestedString
  //- NestedString generates JavaGetNestedString
  //- NestedString generates JavaClearNestedString
  //- NestedBool generates JavaSetNestedBool
  //- NestedBool generates JavaGetNestedBool
  //- NestedBool generates JavaClearNestedBool

  //- OneofString generates JavaSetOneofString
  //- OneofString generates JavaClearOneofString

  //- RepeatedInt32Field generates JavaAddRepeatedInt32Field
  //- RepeatedInt32Field generates JavaClearRepeatedInt32Field
}
