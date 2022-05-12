package pkg;

@SuppressWarnings("unused")
public final class SpecialMembers {
  private SpecialMembers() {}

  public static void main(String[] args) {
    //- @length ref Length
    //- Length.node/kind variable
    System.err.println(args.length);

    //- @clone ref Clone
    //- Clone.node/kind function
    String[] arrayClone = args.clone();

    int[] intArray = new int[]{1, 2, 3};
    //- @clone ref Clone
    intArray = intArray.clone();
    //- @length ref Length
    System.err.println(intArray.length);

    //- @getClass ref GetClass
    //- GetClass.node/kind function
    Class<?> arrayClass = args.getClass();

    //- @class ref CRef
    //- CRef.subkind field
    //- !{ @class ref IntRef }
    System.err.println(SpecialMembers.class);

    //- @class ref IntRef
    //- IntRef.subkind field
    //- !{ @class ref CRef
    //-    @class ref LongRef }
    System.err.println(int.class);

    //- @class ref LongRef
    //- LongRef.subkind field
    //- !{ @class ref IntRef }
    System.err.println(long.class);
  }
}
