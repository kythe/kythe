// Verifies that documentation nodes are emitted for indexed elements

//- @+2"main" defines/binding FnMain
/// The main function
fn main() {
    println!("Hello, world!");
}

//- FnMain.node/kind function
//- FnDoc.text "The main function"
//- FnDoc documents FnMain
