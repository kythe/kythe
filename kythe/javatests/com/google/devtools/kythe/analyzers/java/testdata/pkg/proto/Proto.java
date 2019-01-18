package pkg.proto;

public class Proto {
  static void buildMessage() {
    //- @Message ref JavaMessage
    //- @setStringField ref JavaStringField
    Testdata.Message.newBuilder().setStringField("blah").build();

    //- Message generates JavaMessage
    //- StringField generates JavaStringField
  }
}
