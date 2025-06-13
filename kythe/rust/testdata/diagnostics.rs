#[cfg(kythe_indexer_test)]
mod broken {
    //- @Y tagged Error
    //- Error.node/kind diagnostic
    //- Error.message "no such value in this scope"
    const X: () = Y;
}
