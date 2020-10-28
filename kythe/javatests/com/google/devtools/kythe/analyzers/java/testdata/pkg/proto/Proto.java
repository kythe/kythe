package pkg.proto;

import static pkg.proto.Testdata.Message;

public class Proto {

  //- @Message ref JavaMessage
  static final Message MSG =
      Message.newBuilder()
          //- @setStringField ref JavaSetStringField
          .setStringField("blah")
          //- @setInt32Field ref JavaSetInt32Field
          .setInt32Field(43)
          //- @setNestedMessage ref JavaSetNestedMessageField
          .setNestedMessage(
              //- @NestedMessage ref JavaNestedMessage
              Message.NestedMessage.newBuilder()
                  //- @setNestedString ref JavaSetNestedString
                  .setNestedString("nested")
                  //- @setNestedBool ref JavaSetNestedBool
                  .setNestedBool(true)
                  .build())
          //- @setOneofString ref JavaSetOneofString
          .setOneofString("hello")
          .build();

  static void getFields() {
    //- @getStringField ref JavaGetStringField
    print(MSG.getStringField());
    //- @getInt32Field ref JavaGetInt32Field
    print(MSG.getInt32Field());
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
  //- Int32Field generates JavaSetInt32Field
  //- Int32Field generates JavaGetInt32Field
  //- NestedMessageField generates JavaSetNestedMessageField
  //- NestedMessageField generates JavaGetNestedMessageField

  //- NestedMessage generates JavaNestedMessage
  //- NestedString generates JavaSetNestedString
  //- NestedString generates JavaGetNestedString
  //- NestedBool generates JavaSetNestedBool
  //- NestedBool generates JavaGetNestedBool

  //- OneofString generates JavaSetOneofString
}
