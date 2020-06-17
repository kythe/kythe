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

use protobuf::error::ProtobufError;

quick_error! {
    #[derive(Debug)]
    pub enum KytheError {
        // The FileProvider failed to find the file
        FileNotFoundError {
            display("The requested file could not be found")
        }
        // The FileProvider failed to read the file
        FileReadError(err: std::io::Error) {
            from()
            display("Failed to read contents of file: {}", err)
        }
        // The KzipFileProvider couldn't read the provided file
        KzipFileError(err: zip::result::ZipError) {
            from()
            display("Failed to open kzip: {}", err)
        }
        // There was an error parsing the Protobuf
        ProtobufParseError(err: ProtobufError) {
            from()
            display("Failed to parse Protobuf: {}", err)
        }
        // The KytheWriter encounters an error
        WriterError {}
        // An unknown error occured
        UnknownError(err: Box<dyn std::error::Error>) {
            from()
            display("An unknown error occurred: {}", err)
        }
    }
}
