// Checks that the indexer emits file nodes with content.
mod a;
//- vname("", "", "", "kythe/rust/indexer/testdata/basic/file_content.rs",
//-   "").node/kind file
//- AFile = vname("", "", "", "kythe/rust/indexer/testdata/basic/a.rs", "")
//-   .node/kind file
//- AFile.text "fn main(){}\n"
