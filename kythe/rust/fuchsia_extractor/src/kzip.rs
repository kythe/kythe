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

use {
    crate::kzip_sys as sys,
    analysis_rust_proto::*,
    anyhow::{Context, Result},
    protobuf::Message,
    std::ffi::OsStr,
};

/// The possible encodings.
#[derive(Debug, Clone)]
#[repr(u32)]
pub enum Encoding {
    /// The encoding of the compilation unit will be JSON.
    Json = sys::KZIP_WRITER_ENCODING_JSON,
    /// The encoding of the compilation unit will be protobuf.
    Proto = sys::KZIP_WRITER_ENCODING_PROTO,
}

/// A kzip file writer.
#[derive(Debug)]
pub struct Writer {
    // Internal representation of the writer.
    rep: std::ptr::NonNull<sys::KzipWriter>,
    // If set, close() was called on this Writer.
    closed: bool,
}

impl Drop for Writer {
    fn drop(&mut self) {
        if !self.closed {
            self.close().expect("close failed");
        }
        unsafe { sys::KzipWriter_Delete(self.rep.as_ptr()) };
    }
}

impl Writer {
    /// Creates a new Writer.
    pub fn try_new(path: impl AsRef<OsStr>, encoding: Encoding) -> Result<Writer> {
        let pathstr = path
            .as_ref()
            .to_str()
            .with_context(|| format!("could not convert to str: {:?}", path.as_ref()))?;
        let mut status: i32 = 0;
        let raw: *mut sys::KzipWriter = unsafe {
            sys::KzipWriter_Create(
                pathstr.as_ptr() as *const std::os::raw::c_char,
                pathstr.len() as sys::size_t,
                encoding as i32,
                &mut status,
            )
        };
        if status != 0 {
            return Err(anyhow::anyhow!("could not create KzipWriter: code: {}", status));
        }
        Ok(Writer { rep: std::ptr::NonNull::new(raw).expect("non-null in try_new"), closed: false })
    }

    /// Closes the writer and flushes its buffers.i
    ///
    /// [close] can be called many times, but it takes effect only the first
    /// time around.
    pub fn close(&mut self) -> Result<()> {
        if self.closed {
            // Make multiple closes idempotent.
            return Ok(());
        }
        let status = unsafe { sys::KzipWriter_Close(self.rep.as_ptr()) };
        if status != 0 {
            return Err(anyhow::anyhow!(
                "kzip::Writer::close returned: {} for {:?}",
                status,
                &self
            ));
        }
        self.closed = true;
        Ok(())
    }

    /// Writes the specified file content.  Returns the resulting file digest.
    pub fn write_file(&mut self, content: &[u8]) -> Result<String> {
        const CAPACITY: usize = 200;
        let mut buf: Vec<u8> = vec![0; CAPACITY];
        let mut resulting_buffer_size: sys::size_t = 0;
        let status = unsafe {
            sys::KzipWriter_WriteFile(
                self.rep.as_ptr(),
                content.as_ptr() as *const std::os::raw::c_char,
                content.len() as sys::size_t,
                buf.as_mut_ptr() as *mut std::os::raw::c_char,
                buf.len() as u64,
                &mut resulting_buffer_size,
            )
        };
        if status != 0 {
            return Err(anyhow::anyhow!("kzip::write_file: error code: {}", status));
        }
        // This should always be a UTF-8 clean string since it's a digest.
        buf.resize(resulting_buffer_size as usize, 0);
        Ok(String::from_utf8(buf)?)
    }

    /// Writes the specified compilation unit.  Returns the resulting file
    /// digest.
    pub fn write_unit(&mut self, unit: &IndexedCompilation) -> Result<String> {
        const CAPACITY: usize = 200;
        let mut buf: Vec<u8> = vec![0; CAPACITY];
        let mut resulting_buffer_size: sys::size_t = 0;
        let content =
            unit.write_to_bytes().with_context(|| "kzip::write_unit: while writing protobuf")?;
        let status = unsafe {
            sys::KzipWriter_WriteUnit(
                self.rep.as_ptr(),
                content.as_ptr() as *const std::os::raw::c_char,
                content.len() as sys::size_t,
                buf.as_mut_ptr() as *mut std::os::raw::c_char,
                buf.len() as u64,
                &mut resulting_buffer_size,
            )
        };
        if status != 0 {
            return Err(anyhow::anyhow!(
                "kzip::write_unit: error code: {}, while writing {} bytes",
                status,
                content.len()
            ));
        }
        // This should always be a UTF-8 clean string since it's a digest.
        buf.resize(resulting_buffer_size as usize, 0);
        Ok(String::from_utf8(buf)?)
    }
}

#[cfg(test)]
mod tests {
    use {super::*, std::path::Path, tempdir::TempDir};

    #[test]
    fn check_zip_exists() {
        let temp_dir = TempDir::new("check_zip_exists").expect("temp dir created");
        let output_file = temp_dir.path().join("out.kzip");
        let content = "content";
        let unit = analysis::IndexedCompilation::new();
        assert!(!Path::exists(&output_file), "file exists but should not: {:?}", &output_file);
        {
            let mut writer =
                Writer::try_new(&output_file, Encoding::Proto).expect("created writer");
            writer.write_file(content.as_bytes()).expect("wrote file with success");
            writer.write_unit(&unit).expect("wrote compilation unit with success");
        }
        assert!(Path::exists(&output_file), "file does not exist but should: {:?}", &output_file);
    }
}
