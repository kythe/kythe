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

#![allow(non_camel_case_types)]

pub const KZIP_WRITER_ENCODING_JSON: u32 = 1;
pub const KZIP_WRITER_ENCODING_PROTO: u32 = 2;
pub const KZIP_WRITER_ENCODING_ALL: u32 = 3;
pub const KZIP_WRITER_BUFFER_TOO_SMALL_ERROR: i32 = -1;
pub const KZIP_WRITER_PROTO_PARSING_ERROR: i32 = -2;
pub type size_t = ::std::os::raw::c_ulong;
#[doc = " \\brief The opaque kzip writer object."]
#[repr(C)]
#[derive(Debug, Copy, Clone)]
pub struct KzipWriter {
    _unused: [u8; 0],
}
extern "C" {
    #[doc = " \\brief Creates a new kzip writer."]
    #[doc = " \\param path The path to the file to create."]
    #[doc = " \\param path_len the string length of path, required due to the C API."]
    #[doc = " \\param encoding The compilation unit encoding, either"]
    #[doc = "        KZIP_WRITER_ENCODING_JSON, or KZIP_WRITER_ENCODING_PROTO"]
    #[doc = " \\param create_status zero if created fine, nonzero in case of an error."]
    #[doc = "        Positive error values are the same as in absl::Status.  Negative"]
    #[doc = "        error values are either KZIP_WRITER_BUFFER_TOO_SMALL_ERROR, or"]
    #[doc = "        KZIP_WRITER_PROTO_PARSING_ERROR."]
    #[doc = " Caller takes the ownership of the returned pointer.  KzipWriter_Close must"]
    #[doc = " be called to release the memory."]
    pub fn KzipWriter_Create(
        path: *const ::std::os::raw::c_char,
        path_len: size_t,
        encoding: i32,
        create_status: *mut i32,
    ) -> *mut KzipWriter;
}
extern "C" {
    #[doc = " \\brief Deallocates KzipWriter."]
    #[doc = ""]
    #[doc = " Must be called once KzipWriter is no longer needed."]
    pub fn KzipWriter_Delete(writer: *mut KzipWriter);
}
extern "C" {
    #[doc = " \\brief Closes the writer."]
    #[doc = " Must be called before destroying the object."]
    pub fn KzipWriter_Close(writer: *mut KzipWriter) -> i32;
}
extern "C" {
    #[doc = " \\brief Writes a piece of content into the kzip, returning a digest."]
    #[doc = ""]
    #[doc = " The content is"]
    #[doc = " specified using both the pointer to the beginning and the size.  The caller"]
    #[doc = " must provide a buffer to put the digest into, and a buffer size.  Nonzero"]
    #[doc = " status is returned in case of an error writing."]
    #[doc = " \\param writer The writer to write into."]
    #[doc = " \\param content The file content buffer to write."]
    #[doc = " \\param content_length The length of the buffer to write."]
    #[doc = " \\param digest_buffer The output buffer to store the digest into.  Must be"]
    #[doc = "        large enough to hold a digest."]
    #[doc = " \\param buffer_length The length of digest_buffer in bytes."]
    pub fn KzipWriter_WriteFile(
        writer: *mut KzipWriter,
        content: *const ::std::os::raw::c_char,
        content_length: size_t,
        digest_buffer: *mut ::std::os::raw::c_char,
        buffer_length: size_t,
        resulting_digest_size: *mut size_t,
    ) -> i32;
}
extern "C" {
    #[doc = " The caller must provide a buffer to put the digest into, and a buffer size."]
    #[doc = " Nonzero status is returned in case of an error while writing."]
    pub fn KzipWriter_WriteUnit(
        writer: *mut KzipWriter,
        proto: *const ::std::os::raw::c_char,
        proto_length: size_t,
        digest_buffer: *mut ::std::os::raw::c_char,
        buffer_length: size_t,
        resulting_digest_size: *mut size_t,
    ) -> i32;
}
