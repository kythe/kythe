// Copyright 2020 The Kythe Authors. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

use std::collections::HashMap;

/// Maintains an index of byte offsets for files
pub struct OffsetIndex {
    files: HashMap<String, Vec<LineIndex>>,
}

impl OffsetIndex {
    /// Initialize an empty OffsetIndex
    pub fn new() -> Self {
        Self { files: HashMap::new() }
    }

    /// Adds a new file using the provided string content
    pub fn add_file(&mut self, file_name: &str, file_content: &str) {
        let mut offset = 0;
        let mut line_indices: Vec<LineIndex> = Vec::new();

        for line in file_content.split_inclusive('\n') {
            let line_bytes = line.as_bytes();
            let num_columns = line.chars().count();
            let all_single_byte = num_columns == line_bytes.len();
            let mut column_indices: Vec<u32> = Vec::new();

            if !all_single_byte {
                // Populate the column_indices with the byte offset for each column in the line
                for index in line.char_indices() {
                    column_indices.push(index.0 as u32)
                }
                column_indices.push(line_bytes.len() as u32);
            }

            // Add the new LineIndex and increase the offset
            line_indices.push(LineIndex::new(
                all_single_byte,
                offset as u32,
                num_columns as u32,
                column_indices,
            ));
            offset += line_bytes.len();
        }

        // Add the new file to the HashMap
        self.files.insert(file_name.to_string(), line_indices);
    }

    /// Get the byte offset for a line and column in a file. Returns None if the
    /// file isn't present in the index or if there isn't content at the
    /// requested line/column pair.
    pub fn get_byte_offset(&self, file_name: &str, line: u32, column: u32) -> Option<u32> {
        // Ensure the file is in the index
        if let Some(line_indices) = self.files.get(file_name) {
            // Ensure the line is in bounds
            if line < 1 {
                return None;
            }
            if let Some(line_index) = line_indices.get(line as usize - 1) {
                line_index.get_offset_at_col(column)
            } else {
                None
            }
        } else {
            None
        }
    }
}

impl Default for OffsetIndex {
    fn default() -> Self {
        Self::new()
    }
}

/// Maintains an index for byte offsets in a particular line in a file
pub struct LineIndex {
    // Whether the line has all single byte characters
    all_single_byte: bool,
    // The byte offset to get to this line
    offset: u32,
    // The number of columns
    num_columns: u32,
    // If `all_single_byte` is false, this contains the column byte offset for a character in index
    // `column - 1`. For example, if a line contains at least one multi-byte character
    // (`all_single_byte` == false) then the byte offset for the character in column 7 is
    // `self.offset + column_indices[6]`. If `all_single_byte` is true, then the byte offset would
    // just be `self.offset + 6 - 1`.
    column_indices: Vec<u32>,
}

impl LineIndex {
    /// Create a new instance of LineIndex with the provided values
    pub fn new(
        all_single_byte: bool,
        offset: u32,
        num_columns: u32,
        column_indices: Vec<u32>,
    ) -> Self {
        Self { all_single_byte, offset, num_columns, column_indices }
    }

    /// Get the byte offset of the character on this line at the provided
    /// column. Returns `None` if `column` is greater than the number of columns
    /// in this line, or if `all_single_byte` is false and the index of
    /// `column - 1` isn't in `column_indices`
    pub fn get_offset_at_col(&self, column: u32) -> Option<u32> {
        // Column is past total number of columns
        if column > self.num_columns || column < 1 {
            return None;
        }

        if self.all_single_byte {
            Some(self.offset + column - 1)
        } else {
            // Get column offset from vector if it exists
            self.column_indices
                .get(column as usize - 1)
                .map(|column_offset| self.offset + column_offset)
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    #[test]
    fn single_byte_single_line_works() {
        let mut index = OffsetIndex::new();
        let file_content = "Test string";
        index.add_file("file.txt", file_content);
        // "s" in "string"
        assert_eq!(index.get_byte_offset("file.txt", 1, 6), Some(5));
    }

    #[test]
    fn single_byte_multi_line_lf_works() {
        let mut index = OffsetIndex::new();
        let file_content = "Test string\nNew";
        index.add_file("file.txt", file_content);
        // "N" in "New"
        assert_eq!(index.get_byte_offset("file.txt", 2, 1), Some(12));
    }

    #[test]
    fn single_byte_multi_line_crlf_works() {
        let mut index = OffsetIndex::new();
        let file_content = "Test string\r\nNew";
        index.add_file("file.txt", file_content);
        // "N" in "New"
        assert_eq!(index.get_byte_offset("file.txt", 2, 1), Some(13));
    }

    #[test]
    fn multi_byte_single_line_works() {
        let mut index = OffsetIndex::new();
        let file_content = "🥳 This code works!";
        index.add_file("file.txt", file_content);
        // "T" in "This". An emoji is 4 bytes
        assert_eq!(index.get_byte_offset("file.txt", 1, 3), Some(5));
    }

    #[test]
    fn multi_byte_multi_line_lf_works() {
        let mut index = OffsetIndex::new();
        let file_content = "🥳 This code works!\nNew";
        index.add_file("file.txt", file_content);
        // "N" in "New". An emoji is 4 bytes
        assert_eq!(index.get_byte_offset("file.txt", 2, 1), Some(22));
    }

    #[test]
    fn multi_byte_multi_line_crlf_works() {
        let mut index = OffsetIndex::new();
        let file_content = "🥳 This code works!\r\nNew";
        index.add_file("file.txt", file_content);
        // "N" in "New". An emoji is 4 bytes
        assert_eq!(index.get_byte_offset("file.txt", 2, 1), Some(23));
    }
}
