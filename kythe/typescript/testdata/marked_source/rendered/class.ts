
//- @MyClass defines/binding MyClass
//- MyClass.code/rendered/qualified_name "MyClass"
class MyClass {
    //- @myMethod defines/binding MyMethod
    //- MyMethod.code/rendered/callsite_signature "myMethod(arg)"
    //- MyMethod.code/rendered/signature "myMethod(arg: number): MyClass"
    //- MyMethod.code/rendered/qualified_name "MyClass.myMethod"
    myMethod(arg: number): MyClass {
        return this;
    }
}
