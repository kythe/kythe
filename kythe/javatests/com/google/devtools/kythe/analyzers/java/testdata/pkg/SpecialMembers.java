package pkg;

public final class SpecialMembers {
  public static void main(String[] args) {
    //- @length ref Length
    //- Length.node/kind variable
    System.err.println(args.length);

    //- @clone ref Clone
    //- Clone.node/kind function
    String[] arrayClone = args.clone();

    int[] intArray = new int[]{1, 2, 3};
    //- @clone ref Clone
    intArray.clone();
    //- @length ref Length
    System.err.println(intArray.length);

    //- @getClass ref GetClass
    //- GetClass.node/kind function
    Class<?> arrayClass = args.getClass();

    // TODO(schroederc): @class ref Class
    System.err.println(SpecialMembers.class);

    // TODO(schroederc): @class ref Class
    System.err.println(int.class);
  }
}
