// aliases/root and aliases behave as we expect.

template <typename Named>
struct Aliaser {
  //- @Type defines/binding AliaserType
  using Type = Named;
};

//- @Root defines/binding RootAlias
//- RootAlias aliases/root CharBuiltin
//- CharBuiltin.node/kind tbuiltin
using Root = char;

//- @Type ref AliaserType
//- @First defines/binding FirstAlias
//- FirstAlias aliases AliaserType
//- FirstAlias aliases/root CharBuiltin
using First = typename Aliaser<Root>::Type;

//- @Type ref AliaserType
//- @Second defines/binding SecondAlias
//- SecondAlias aliases AliaserType
//- SecondAlias aliases/root CharBuiltin
using Second = typename Aliaser<First>::Type;

//- @Type ref AliaserType
//- @Third defines/binding ThirdAlias
//- ThirdAlias aliases AliaserType
//- ThirdAlias aliases/root CharBuiltin
using Third = typename Aliaser<Second>::Type;
