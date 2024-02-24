package pkg;

// Check that variable and class member writes are properly marked as ref/writes

@SuppressWarnings("unused")
public class Writes {
  private Writes() {
    // - @a defines/binding A
    int a = 1;
    // - @sc defines/binding SC
    Subclass sc = new Subclass();

    //
    // Test JCAssign
    //

    // - @a ref/writes A
    // - !{ @a ref A }
    a = 2;

    // - @#0a ref/writes A
    // - !{ @#0a ref A }
    // - @#1a ref A
    // - !{ @#1a ref/writes A }
    a = a + 1;

    // - @b defines/binding B
    // - @a ref A
    // - !{ @a ref/writes A }
    int b = a;

    // - @c defines/binding C
    int c = 2;

    // - @sc ref SC
    // - !{ @sc ref/writes SC }
    // - @a ref/writes MemberA
    // - !{ @a ref MemberA }
    sc.a = 2;

    // - @#0a ref/writes MemberA
    // - !{ @#0a ref MemberA }
    // - @#1a ref MemberA
    // - !{ @#1a ref/writes MemberA }
    sc.a = sc.a + 1;

    // - @a ref MemberA
    // - !{ @a ref/writes MemberA }
    b = sc.a;

    // - @arr defines/binding Arr
    int[] arr = {1};

    // - @arr ref Arr
    a = arr[0];

    // - @arr ref Arr
    // - !{ @arr ref/writes Arr }
    arr[0] = 1;

    //
    // Test JCAssignOp
    //

    // - @a ref A
    // - @a ref/writes A
    a += 1;

    // - @a ref MemberA
    // - @a ref/writes MemberA
    sc.a += 1;

    //
    // Test chained writes and writes on the right-hand side
    //

    // - !{ @a ref A }
    // - @a ref/writes A
    // - @b ref B
    // - @b ref/writes B
    // - @c ref C
    // - @c ref/writes C
    a = b = c++;

    // - @a ref A
    // - @a ref/writes A
    // - @#0c ref C
    // - @#0c ref/writes C
    // - @#1c ref C
    // - !{ @#1c ref/writes C }
    a += c++ + 1 + ~c;

    // - @#0a ref/writes A
    // - @#1a ref A
    // - @#1a ref/writes A
    a = (a++ + 1) + 1;

    //
    // Test unary operations
    //

    // - @a ref A
    // - @a ref/writes A
    a++;
    // - @a ref A
    // - @a ref/writes A
    a--;
    // - @a ref A
    // - @a ref/writes A
    ++a;
    // - @a ref A
    // - @a ref/writes A
    --a;

    // - @a ref/writes MemberA
    sc.a++;
    // - @a ref/writes MemberA
    sc.a--;
    // - @a ref/writes MemberA
    ++sc.a;
    // - @a ref/writes MemberA
    --sc.a;

    //
    // Test chained selects
    //

    // - @sub ref MemberSub
    // - !{ @sub ref/writes MemberSub }
    // - @#0a ref/writes MemberA
    // - @#1a ref MemberA
    sc.sub.a = sc.a;
    // - @a ref MemberA
    // - !{ @a ref/writes MemberA }
    b = sc.sub.a;
  }

  class Subclass {
    // - @a defines/binding MemberA
    public int a = 1;

    // - @sub defines/binding MemberSub
    public Subclass sub = new Subclass();

    public Subclass() {}

    // - @x defines/binding X
    public void inc(int x) {
      // - @#0a ref/writes MemberA
      // - !{ @#0a ref MemberA }
      // - @#1a ref MemberA
      // - !{ @#1a ref/writes MemberA }
      // - @x ref X
      this.a = this.a + x;
    }
  }
}
