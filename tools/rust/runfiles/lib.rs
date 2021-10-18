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
        match std::env::var("RUNFILES_DIR") {
            Ok(p) => Ok(Self { runfiles_path: PathBuf::from(p) }),
            Err(_) => Err(Error::new(ErrorKind::NotFound, "Failed to find runfiles directory")),
        }
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
