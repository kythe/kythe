/// Tests that references to ps var templates go to the ps and not the primary.
//- @v defines Primary
template <typename T, typename S> T v = S();
//- @v defines Partial
//- Partial.node/kind abs
//- @v defines PartialVar
//- PartialVar.node/kind variable
template <typename U> U v<U, int> = 3;
//- @v ref Spec
int w = v<int,int>;
//- Spec.node/kind variable
//- Spec instantiates TAppPartial
//- TAppPartial param.0 Partial
