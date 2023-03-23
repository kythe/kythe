//- MainModule=vname("module", _, _, "testdata/tsx_group/main", _).node/kind record

//- @createDiv defines/binding CreateDiv
//- @divId defines/binding IdArgument
export function createDiv(divId) {
    //- @divId ref IdArgument
    return <div id={divId}></div>
}
