package pkg;

public final class SpecialMembers {
  public static void main(String[] args) {
    // TODO(schroederc): handle special members better than not crashing

    //- @length ref Length
    //- Length.node/kind diagnostic
    System.err.println(args.length);

    //- @clone ref Clone
    //- Clone.node/kind diagnostic
    String[] arrayClone = args.clone();

    //- @getClass ref GetClass
    //- GetClass.node/kind function
    Class<?> arrayClass = args.getClass();

    // TODO(schroederc): @class ref Class
    System.err.println(SpecialMembers.class);

    // TODO(schroederc): @class ref Class
    System.err.println(int.class);
  }
}
