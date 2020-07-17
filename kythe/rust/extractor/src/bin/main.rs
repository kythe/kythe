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
#[macro_use]
extern crate anyhow;

mod cli;
mod save_analysis;

use analysis_rust_proto::*;
use anyhow::{Context, Result};
use crypto::{digest::Digest, sha2::Sha256};
use extra_actions_base_rust_proto::*;
use protobuf::Message;
use std::fs::File;
use std::io::{Read, Write};
use std::path::{Path, PathBuf};
use tempdir::TempDir;
use zip::{write::FileOptions, ZipWriter};

fn main() -> Result<()> {
    let config = cli::parse_arguments();

    // Open the .xa file
    let mut extra_action_file = File::open(&config.extra_action_path)
        .with_context(|| format!("Failed to open file {:?}", config.extra_action_path))?;
    let mut extra_action_bytes = Vec::new();
    extra_action_file
        .read_to_end(&mut extra_action_bytes)
        .with_context(|| format!("Failed to read file {:?}", config.extra_action_path))?;

    // Parse as an ExtraActionInfo
    let extra_action = protobuf::parse_from_bytes::<ExtraActionInfo>(&extra_action_bytes)
        .with_context(|| {
            format!("Failed to parse protobuf from file {:?}", config.extra_action_path)
        })?;
    println!("{:?}", extra_action);

    // Retrieve the SpawnInfo extension
    let spawn_info = exts::SPAWN_INFO
        .get(&extra_action)
        .ok_or_else(|| anyhow!("SpawnInfo extension missing"))?;

    // Collect arguments
    let all_arguments: Vec<String> = spawn_info.get_argument().to_vec();

    // Create temporary directory and run the analysis
    let tmp_dir = TempDir::new("rust_extractor")
        .with_context(|| format!("Failed to make temporary directory"))?;
    save_analysis::run_analysis(all_arguments.clone(), PathBuf::new().join(tmp_dir.path()))?;

    // Collect input files
    let all_input_files = spawn_info.get_input_file().to_vec();
    let rust_source_files: Vec<String> = all_input_files
        .iter()
        .filter(|file| file.ends_with(".rs"))
        .map(|file| String::from(file)) // Map the &String to a new String
        .collect();

    // Create compilation unit for kzip
    let mut compilation_unit = CompilationUnit::new();

    // Set basic fields for compilation unit
    let mut unit_vname = VName::new();
    unit_vname.set_corpus("kythe".into());
    unit_vname.set_language("rust".into());
    compilation_unit.set_v_name(unit_vname);

    compilation_unit.set_source_file(protobuf::RepeatedField::from_vec(rust_source_files.clone()));
    compilation_unit.set_argument(protobuf::RepeatedField::from_vec(all_arguments.clone()));

    let build_target_output = spawn_info
        .get_output_file()
        .get(0)
        .ok_or_else(|| anyhow!("Failed to get output file from spawn info"))?;
    compilation_unit.set_output_key(build_target_output.into());

    // Create the output kzip
    let output_file = File::create(&config.output_path)
        .with_context(|| format!("Failed to create kzip file at path {:?}", config.output_path))?;
    let mut kzip = ZipWriter::new(output_file);

    // Loop through each source file to generate vname and insert into kzip
    let mut required_inputs = Vec::new();
    for file_path in rust_source_files {
        let mut source_file = File::open(&file_path)
            .with_context(|| format!("Failed open file {:?}", file_path))?;

        // Calculate sha256 digest
        let mut sha256 = Sha256::new();
        let mut source_file_contents: Vec<u8> = Vec::new();
        source_file.read_to_end(&mut source_file_contents)
            .with_context(|| format!("Failed read file {:?}", file_path))?;
        sha256.input(source_file_contents.as_slice());
        let digest = sha256.result_str().to_owned();

        // Write file to kzip
        kzip.start_file(format!("files/{}", digest), FileOptions::default())
            .with_context(|| format!("Failed create file in kzip: {:?}", file_path))?;
        kzip.write(source_file_contents.as_slice())
            .with_context(|| format!("Failed write file contents to kzip: {:?}", file_path))?;

        // Produce RequiredInput
        let mut file_input = CompilationUnit_FileInput::new();

        // TODO(wcalandro): Use vnames_config to generate vname
        let mut file_vname = VName::new();
        file_vname.set_corpus("kythe".to_string());
        file_vname.set_path(file_path.clone());
        file_input.set_v_name(file_vname);

        let mut file_info = FileInfo::new();
        file_info.set_path(file_path.clone());
        file_info.set_digest(digest.clone());
        file_input.set_info(file_info);

        required_inputs.push(file_input);
    }

    // Add save_analysis to kzip
    // TODO: Abstract adding file to kzip with function

    // Generate name for save analysis file
    let build_target_path = Path::new(build_target_output);
    let analysis_file_name = build_target_path.with_extension("json");
    let analysis_file_str = analysis_file_name.file_name().unwrap().to_str().ok_or_else(|| anyhow!("Failed to convert path to string"))?;

    let mut save_analysis = File::open(tmp_dir.path().join("save-analysis").join(analysis_file_str))
        .with_context(|| format!("Failed open save analysis file"))?;
    let mut save_analysis_contents: Vec<u8> = Vec::new();
    save_analysis.read_to_end(&mut save_analysis_contents)
        .with_context(|| format!("Failed read save analysis file"))?;
    let mut sha256 = Sha256::new();
    sha256.input(save_analysis_contents.as_slice());
    let digest = sha256.result_str().to_owned();
    kzip.start_file(format!("files/{}", digest), FileOptions::default())
            .with_context(|| format!("Failed create save analysis file in kzip"))?;
    kzip.write(save_analysis_contents.as_slice())
            .with_context(|| format!("Failed write save analysis file contents to kzip"))?;
    let mut file_input = CompilationUnit_FileInput::new();
    let mut file_vname = VName::new();
    file_vname.set_corpus("kythe".to_string());
    file_vname.set_path(analysis_file_str.into());
    file_input.set_v_name(file_vname);
    let mut file_info = FileInfo::new();
    file_info.set_path(analysis_file_str.into());
    file_info.set_digest(digest.clone());
    file_input.set_info(file_info);
    required_inputs.push(file_input);

    // Set required_input field
    compilation_unit.set_required_input(protobuf::RepeatedField::from_vec(required_inputs));

    // Convert compilation unit to bytes and write to kzip
    let mut compilation_unit_bytes = Vec::new();
    compilation_unit.write_to_vec(&mut compilation_unit_bytes)
        .with_context(|| format!("Failed write compilation unit to bytes"))?;
    sha256.reset();
    sha256.input(compilation_unit_bytes.as_slice());
    let cu_digest = sha256.result_str().to_owned();

    kzip.start_file(format!("pbunits/{}", cu_digest), FileOptions::default())
            .with_context(|| format!("Failed create compilation unit file in kzip"))?;
    kzip.write(compilation_unit_bytes.as_slice())
            .with_context(|| format!("Failed write compilation unit to kzip"))?;

    Ok(())
}

// Inspired by https://gist.github.com/Igosuki/27e18369fec5a05d4019ac5d321e1779
pub mod exts {
    pub const SPAWN_INFO: protobuf::ext::ExtFieldOptional<
        super::ExtraActionInfo,
        protobuf::types::ProtobufTypeMessage<super::SpawnInfo>,
    > = protobuf::ext::ExtFieldOptional { field_number: 1003, phantom: ::std::marker::PhantomData };
}
