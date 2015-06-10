// We index base classes.
// --ignore_dups=true (T28)
//- @A defines ClassA
class A { };
//- @B defines ClassB ClassB extends/private ClassA
class B : A { };
//- @C defines ClassC ClassC extends/public ClassA
class C : public A { };
//- @D defines ClassD ClassD extends/private ClassA
class D : private A { };
//- @E defines ClassE ClassE extends/protected ClassA
class E : protected A { };
//- @F defines ClassF ClassF extends/private/virtual ClassA
class F : virtual A { };
//- @G defines ClassG ClassG extends/public/virtual ClassA
class G : virtual public A { };
//- @H defines ClassH ClassH extends/private/virtual ClassA
class H : virtual private A { };
//- @I defines ClassI ClassI extends/protected/virtual ClassA
class I : virtual protected A { };
//- @J defines ClassJ
//- ClassJ extends/public ClassC
//- ClassJ extends/private ClassD
class J : public C, private D { };
