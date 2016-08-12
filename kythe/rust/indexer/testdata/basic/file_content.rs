// Checks that the indexer emits file nodes with content.
mod a;
//- vname("", "file_content", "", "file_content.rs",
//-   "").node/kind file
//- AFile = vname("", "file_content", "", "a.rs", "")
//-   .node/kind file
//- AFile.text "fn main(){}\n"
