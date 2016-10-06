// We index base classes.
//- @A defines/binding ClassA
class A { };
//- @B defines/binding ClassB ClassB extends/private ClassA
class B : A { };
//- @C defines/binding ClassC ClassC extends/public ClassA
class C : public A { };
//- @D defines/binding ClassD ClassD extends/private ClassA
class D : private A { };
//- @E defines/binding ClassE ClassE extends/protected ClassA
class E : protected A { };
//- @F defines/binding ClassF ClassF extends/private/virtual ClassA
class F : virtual A { };
//- @G defines/binding ClassG ClassG extends/public/virtual ClassA
class G : virtual public A { };
//- @H defines/binding ClassH ClassH extends/private/virtual ClassA
class H : virtual private A { };
//- @I defines/binding ClassI ClassI extends/protected/virtual ClassA
class I : virtual protected A { };
//- @J defines/binding ClassJ
//- ClassJ extends/public ClassC
//- ClassJ extends/private ClassD
class J : public C, private D { };
