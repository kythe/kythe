package pkg;

// Check that variable and class member writes are properly marked as ref/writes

@SuppressWarnings("unused")
public class Writes {
  private Writes() {
    //- @a defines/binding A
    int a = 1;
    //- @sc defines/binding SC
    Subclass sc = new Subclass();

    //
    // Test JCAssign
    //

    //- @a ref/writes A
    //- !{ @a ref A }
    a = 2;

    //- @#0a ref/writes A
    //- !{ @#0a ref A }
    //- @#1a ref A
    //- !{ @#1a ref/writes A }
    a = a + 1;

    //- @a ref A
    //- !{ @a ref/writes A }
    int b = a;

    //- @sc ref SC
    //- !{ @sc ref/writes SC }
    //- @a ref/writes MemberA
    //- !{ @a ref MemberA }
    sc.a = 2;

    //- @#0a ref/writes MemberA
    //- !{ @#0a ref MemberA }
    //- @#1a ref MemberA
    //- !{ @#1a ref/writes MemberA }
    sc.a = sc.a + 1;

    //- @a ref MemberA
    //- !{ @a ref/writes MemberA }
    b = sc.a;

    //
    // Test JCAssignOp
    //

    //- @a ref A
    //- @a ref/writes A
    a += 1;

    //- @a ref MemberA
    //- @a ref/writes MemberA
    sc.a += 1;

    //
    // Test unary operations
    //

    //- @a ref A
    //- @a ref/writes A
    a++;
    //- @a ref A
    //- @a ref/writes A
    a--;
    //- @a ref A
    //- @a ref/writes A
    ++a;
    //- @a ref A
    //- @a ref/writes A
    --a;

    //- @a ref/writes MemberA
    sc.a++;
    //- @a ref/writes MemberA
    sc.a--;
    //- @a ref/writes MemberA
    ++sc.a;
    //- @a ref/writes MemberA
    --sc.a;

    //
    // Test chained selects
    //

    //- @sub ref MemberSub
    //- !{ @sub ref/writes MemberSub }
    //- @#0a ref/writes MemberA
    //- @#1a ref MemberA
    sc.sub.a = sc.a;
    //- @a ref MemberA
    //- !{ @a ref/writes MemberA }
    b = sc.sub.a;
  }

  class Subclass {
    //- @a defines/binding MemberA
    public int a = 1;

    //- @sub defines/binding MemberSub
    public Subclass sub = new Subclass();

    public Subclass() {}

    public void inc(int x) {
      //- @#0a ref/writes MemberA
      //- !{ @#0a ref MemberA }
      //- @#1a ref MemberA
      //- !{ @#1a ref/writes MemberA }
      this.a = this.a + x;
    }
  }
}
