// Copyright 2021 The Kythe Authors. All rights reserved.
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

use std::io::{Error, ErrorKind};
use std::path::PathBuf;

pub struct Runfiles {
    pub runfiles_path: PathBuf,
}

impl Runfiles {
    pub fn create() -> std::io::Result<Self> {
        let executable_path = std::env::current_exe()?.canonicalize()?;
        // Unwrap is okay because we know the executable path is a file path
        let executable_name_os_str = executable_path.file_name().unwrap();
        // OsStr -> Cow<'_, str> -> String
        let executable_name = executable_name_os_str.to_string_lossy().to_string();

        // See if there is an ${executable_path}.runfiles directory
        // If so, that's the runfiles directory
        let local_path = executable_path.with_file_name(format!("{}.runfiles/", executable_name));
        if local_path.exists() {
            return Ok(Self { runfiles_path: local_path });
        } else {
            // We are probably a test binary and the runfiles directory is one of the parent
            // folders
            let mut current_path = executable_path;
            while current_path.pop() {
                if let Some(dir_name_os) = current_path.file_name() {
                    let dir_name = dir_name_os.to_string_lossy();
                    if dir_name.starts_with(&executable_name) && dir_name.ends_with(".runfiles") {
                        return Ok(Self { runfiles_path: current_path });
                    }
                } else {
                    break;
                }
            }
        }
        Err(Error::new(ErrorKind::NotFound, "Failed to find runfiles directory"))
    }

    /// Returns the full path of a file inside the runfiles directory
    pub fn rlocation(&self, path: &str) -> PathBuf {
        self.runfiles_path.join(path)
    }
}

#[cfg(test)]
mod test {
    use super::Runfiles;

    #[test]
    fn test_path_generated_correctly() {
        let r = Runfiles::create().expect("Failed to get path for current executable");
        let file_path = r.rlocation("io_kythe/tools/rust/runfiles/testfile.txt");
        assert_eq!(file_path.exists(), true);
    }
}
