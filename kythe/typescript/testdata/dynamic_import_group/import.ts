
function regularFunction() {
    //- @"'./module'" ref/imports Mod
    //- @StuffDoer ref/id StuffDoer
    import('./module').then(({StuffDoer}) => {
        new StuffDoer().doStuff();
    })

    // Supported by JS but not working in TS.
    // Make sure that TS indexer doesn't crash.
    import('./modu' + 'le').then(({StuffDoer}) => {
        new StuffDoer().doStuff();
    })
}

async function asyncFunction() {
    const {
        //- @StuffDoer ref/id StuffDoer
        StuffDoer,
        //- @CONSTANT ref/id Constant
        CONSTANT,
        //- @doStuff ref/id DoStuff
        doStuff,
        //- @Enum ref/id Enum
        Enum,
    //- @"'./module'" ref/imports Mod
    } = await import('./module');
    new StuffDoer().doStuff();
}

