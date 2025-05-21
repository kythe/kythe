// Names in distinct namespaces must have distinct vnames.
//- @foo defines/binding TypeNs
//- TypeNs.node/kind record
//- !{ @foo defines/binding ValueNs }
struct foo {}
//- @foo defines/binding ValueNs
//- ValueNs.node/kind function
//- !{ @foo defines/binding TypeNs }
fn foo() {}

// Children in the same namespace must still have distinct vnames if their respective parents are
// in different namespaces.
mod bar {
    //- @T defines/binding ParentInTypeNs
    //- ParentInTypeNs.node/kind talias
    //- !{ @T defines/binding ParentInValueNs }
    type T = ();
}
//- @T defines/binding ParentInValueNs
//- ParentInValueNs.node/kind tvar
//- !{ @T defines/binding ParentInTypeNs }
fn bar<T>() {}

// While fields don't technically live in an explicit namespace, we pretend that they do to avoid
// vname conflicts.
//- @U defines/binding NotFieldNs
//- NotFieldNs.node/kind tvar
//- !{ @U defines/binding FieldNs }
struct Baz<U> {
    //- @U defines/binding FieldNs
    //- FieldNs.node/kind variable
    //- !{ @U defines/binding NotFieldNs }
    U: u8,
    // Compiler wants us to use the type variable somewhere.
    _phantom: std::marker::PhantomData<U>,
}
