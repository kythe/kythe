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

use clap::{App, Arg};
use std::path::PathBuf;

/// Contains the configuration options for the extractor
pub struct ExtractorConfig {
    pub extra_action_path: PathBuf,
    pub vnames_config_path: PathBuf,
    pub output_path: PathBuf,
}

/// Parse the command line arguments into an `ExtractorConfig`
pub fn parse_arguments() -> ExtractorConfig {
    let matches = App::new("Kythe Rust Extractor")
        .arg(
            Arg::with_name("extra_action")
                .long("extra_action")
                .required(true)
                .takes_value(true)
                .help("Path of the extra action file"),
        )
        .arg(
            Arg::with_name("output")
                .long("output")
                .required(true)
                .takes_value(true)
                .help("Desired output path for the kzip"),
        )
        .arg(
            Arg::with_name("vnames_config")
                .long("vnames_config")
                .required(true)
                .takes_value(true)
                .help("Location of vnames configuration file"),
        )
        .get_matches();

    let extra_action_path = PathBuf::from(matches.value_of("extra_action").unwrap());
    let vnames_config_path = PathBuf::from(matches.value_of("vnames_config").unwrap());
    let output_path = PathBuf::from(matches.value_of("output").unwrap());
    ExtractorConfig { extra_action_path, vnames_config_path, output_path }
}
