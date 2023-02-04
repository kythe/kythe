
function regularFunction() {
    //- @"'./module'" ref/imports Mod
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
    //- @"'./module'" ref/imports Mod
    const {StuffDoer} = await import('./module');
    new StuffDoer().doStuff();
}

