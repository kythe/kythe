class WritesClass {
  //- @member defines/binding Member
  member: number;

  //- @sub defines/binding MemberSub
  sub: WritesClass;

  constructor() {
    //- !{ @member ref Member }
    //- @member ref/writes Member
    this.member = 1;
    //- !{ @sub ref MemberSub }
    //- @sub ref/writes MemberSub
    this.sub = new WritesClass();
  }
}

//
// Test Assignments
//

//- @w defines/binding W
let w = 1;

//- !{ @w ref W }
//- @w ref/writes W
w = 2;

//- !{ @#0w ref W }
//- @#0w ref/writes W
//- @#1w ref W
//- !{ @#1w ref/writes W }
w = w + 2;

//- @x defines/binding X
let x = 2;

//- !{ @x ref X }
//- @x ref/writes X
//- @w ref W
//- @w ref/writes W
x = w = 3;

//- !{ @#0x ref X }
//- @#0x ref/writes X
//- @#1x ref X
//- @#1x ref/writes X
x = x++ + 1;

//- @y defines/binding Y
let y = new WritesClass();

//- @y ref Y
//- !{ @y ref/writes Y }
//- !{ @member ref Member }
//- @member ref/writes Member
y.member = 1;

//- !{ @#0member ref Member }
//- @#0member ref/writes Member
//- @#1member ref Member
//- !{ @#1member ref/writes Member }
y.member = y.member + 1;

//
// Test pre/postfix operations
//

//- @w ref W
//- @w ref/writes W
w++;
//- @w ref W
//- @w ref/writes W
w--;
//- @w ref W
//- @w ref/writes W
++w;
//- @w ref W
//- @w ref/writes W
--w;

//- @member ref Member
//- @member ref/writes Member
y.member++;
//- @member ref Member
//- @member ref/writes Member
y.member--;
//- @member ref Member
//- @member ref/writes Member
++y.member;
//- @member ref Member
//- @member ref/writes Member
--y.member;

//
// Test compound assignment token
//

//- @w ref W
//- @w ref/writes W
w += 1;
//- @w ref W
//- @w ref/writes W
w -= 1;
//- @w ref W
//- @w ref/writes W
w *= 1;
//- @w ref W
//- @w ref/writes W
w **= 1;
//- @w ref W
//- @w ref/writes W
w /= 1;
//- @w ref W
//- @w ref/writes W
w %= 1;
//- @w ref W
//- @w ref/writes W
w <<= 1;
//- @w ref W
//- @w ref/writes W
w >>= 1;
//- @w ref W
//- @w ref/writes W
w >>>= 1;
//- @w ref W
//- @w ref/writes W
w &= 1;
//- @w ref W
//- @w ref/writes W
w |= 1;
//- @w ref W
//- @w ref/writes W
w ^= 1;

//
// Test chained property access
//

//- @sub ref MemberSub
//- !{ @sub ref/writes MemberSub }
//- !{ @member ref Member }
//- @member ref/writes Member
y.sub.member = 1;

//
// Some more complicated cases
//

//- @z defines/binding Z
let z = 1;

//- !{ @z ref Z }
//- @z ref/writes Z
//- @x ref X
//- @x ref/writes X
//- @#0w ref W
//- @#0w ref/writes W
//- @#1w ref W
//- !{ @#1w ref/writes W }
z = x = w++ + w + 1;

//- @#0z ref Z
//- @#0z ref/writes Z
//- @#1z ref Z
//- !{ @#1z ref/writes Z }
z += z;
