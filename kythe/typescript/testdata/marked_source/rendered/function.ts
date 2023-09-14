
//- @myFunction defines/binding MyFunction
//- MyFunction.code/rendered/qualified_name "myFunction"
//- MyFunction.code/rendered/callsite_signature "myFunction(arg)"
//- MyFunction.code/rendered/signature "myFunction(arg: string): number"
function myFunction(arg: string = '0'): number {
    return 0;
}

//- @varArgs defines/binding VarArgs
//- VarArgs.code/rendered/qualified_name "varArgs"
//- VarArgs.code/rendered/callsite_signature "varArgs(arg)"
//- VarArgs.code/rendered/signature "varArgs(...arg: any[]): void"
function varArgs(...arg: any[]) {}
