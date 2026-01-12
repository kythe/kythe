// Deps:
//   rust_analyzer

use rust_analyzer::{
    base_db::FileId,
    hir::{FileRangeWrapper, InFileWrapper},
    ide::TextRange,
};

/// Helper to unify various source-range representations rust-analyzer uses.
#[derive(Debug, Clone, Copy)]
pub struct FileRange {
    pub file_id: FileId,
    pub start: u32,
    pub end: u32,
}

impl FileRange {
    pub fn new(file_id: FileId, range: TextRange) -> FileRange {
        FileRange { file_id, start: range.start().into(), end: range.end().into() }
    }
}

impl From<FileRangeWrapper<FileId>> for FileRange {
    fn from(value: FileRangeWrapper<FileId>) -> FileRange {
        FileRange::new(value.file_id, value.range)
    }
}

impl From<InFileWrapper<FileId, TextRange>> for FileRange {
    fn from(value: InFileWrapper<FileId, TextRange>) -> FileRange {
        FileRange::new(value.file_id, value.value)
    }
}
