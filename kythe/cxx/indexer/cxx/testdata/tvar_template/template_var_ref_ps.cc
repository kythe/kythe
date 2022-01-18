/// Tests that references to ps var templates go to the ps and not the primary.
//- @v defines/binding Primary
template <typename T, typename S> T v = S();
//- @v defines/binding PartialVar
//- PartialVar.node/kind variable
template <typename U> U v<U, int> = 3;
//- @v ref Spec
int w = v<int,int>;
//- Spec.node/kind variable
//- Spec instantiates TAppPartialVar
//- TAppPartialVar param.0 PartialVar
