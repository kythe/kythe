
function takeAnything(a: any) {}

takeAnything(
    //- @"() => {}" defines vname("arrow0", _, _, _, _)
    () => {}
);

takeAnything(
    //- @"() => {}" defines vname("arrow1", _, _, _, _)
    () => {}
);

takeAnything(
    //- @"function() {}" defines vname("func2", _, _, _, _)
    function() {}
)
