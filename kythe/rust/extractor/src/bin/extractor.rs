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
use extra_actions_base_rust_proto::*;
use kythe_rust_extractor::vname_util::VNameRule;
use protobuf::Message;
use sha2::{Digest, Sha256};
use std::fs::File;
use std::io::{Read, Write};
use std::path::{Path, PathBuf};
use tempdir::TempDir;
use zip::{write::FileOptions, ZipWriter};

fn main() -> Result<()> {
    let config = cli::parse_arguments();

    // Parse the VName configuration rules
    let mut vname_rules = VNameRule::parse_vname_rules(config.vnames_config_path.as_path())?;

    // Retrieve the SpawnInfo from the extra action file
    let spawn_info = get_spawn_info(&config.extra_action_path)?;

    // Set environment variables from spawn info
    let environment_variables = spawn_info.get_variable().to_vec();
    let pwd = std::env::current_dir().context("Couldn't determine pwd")?;
    for variable in environment_variables {
        let value = variable.get_value().replace("${pwd}", pwd.to_str().unwrap());
        std::env::set_var(variable.get_name(), value);
    }

    // Create temporary directory and run the analysis
    let tmp_dir = TempDir::new("rust_extractor")
        .with_context(|| "Failed to make temporary directory".to_string())?;
    let build_target_arguments: Vec<String> = spawn_info.get_argument().to_vec();
    save_analysis::generate_save_analysis(
        build_target_arguments.clone(),
        PathBuf::from(tmp_dir.path()),
    )?;

    // Create the output kzip
    let kzip_file = File::create(&config.output_path)
        .with_context(|| format!("Failed to create kzip file at path {:?}", config.output_path))?;
    let mut kzip = ZipWriter::new(kzip_file);
    kzip.add_directory("root/", FileOptions::default())?;

    // See if the KYTHE_CORPUS variable is set
    let default_corpus = std::env::var("KYTHE_CORPUS").unwrap_or_default();

    // Loop through each source file and insert into kzip
    // Collect input files
    let rust_source_files: Vec<String> = spawn_info
        .get_input_file()
        .to_vec()
        .iter()
        .filter(|file| file.ends_with(".rs"))
        .map(String::from) // Map the &String to a new String
        .collect();
    let mut required_input: Vec<CompilationUnit_FileInput> = Vec::new();
    for file_path in &rust_source_files {
        // Generate the file vname based on the vname rules
        let file_vname: VName = create_vname(&mut vname_rules, file_path, &default_corpus);
        kzip_add_required_input(file_path, file_vname, &mut kzip, &mut required_input)?;
    }

    // Grab the build target's output path
    let build_output_path: &String = spawn_info
        .get_output_file()
        .get(0)
        .ok_or_else(|| anyhow!("Failed to get output file from spawn info"))?;

    // Add save analysis to kzip
    let save_analysis_path: String = analysis_path_string(build_output_path, tmp_dir.path())?;
    let save_analysis_vname: VName =
        create_vname(&mut vname_rules, &save_analysis_path, &default_corpus);
    kzip_add_required_input(
        &save_analysis_path,
        save_analysis_vname,
        &mut kzip,
        &mut required_input,
    )?;

    // Create the IndexedCompilation and add it to the kzip
    let main_source_path = &rust_source_files[0];
    let mut unit_vname = create_vname(&mut vname_rules, main_source_path, &default_corpus);
    if !default_corpus.is_empty() {
        unit_vname.set_corpus(default_corpus);
    }
    let indexed_compilation = create_indexed_compilation(
        rust_source_files,
        build_target_arguments,
        build_output_path,
        required_input,
        unit_vname,
    );
    let mut indexed_compilation_bytes: Vec<u8> = Vec::new();
    indexed_compilation
        .write_to_vec(&mut indexed_compilation_bytes)
        .with_context(|| "Failed to serialize IndexedCompilation to bytes".to_string())?;
    let indexed_compilation_digest = sha256digest(&indexed_compilation_bytes);
    kzip_add_file(
        format!("root/pbunits/{}", indexed_compilation_digest),
        &indexed_compilation_bytes,
        &mut kzip,
    )?;

    Ok(())
}

// Inspired by https://gist.github.com/Igosuki/27e18369fec5a05d4019ac5d321e1779
const SPAWN_INFO_FIELD_NUMBER: u32 = 1003;
const SPAWN_INFO: protobuf::ext::ExtFieldOptional<
    ExtraActionInfo,
    protobuf::types::ProtobufTypeMessage<SpawnInfo>,
> = protobuf::ext::ExtFieldOptional {
    field_number: SPAWN_INFO_FIELD_NUMBER,
    phantom: ::std::marker::PhantomData,
};

/// Attempt to parse ExtraActionInfo protobuf from file_path and extract the
/// SpawnInfo extension
fn get_spawn_info(file_path: impl AsRef<Path>) -> Result<SpawnInfo> {
    let mut file = File::open(file_path).context("Failed to open extra action file")?;

    let mut file_contents_bytes = Vec::new();
    file.read_to_end(&mut file_contents_bytes).context("Failed to read extra action file")?;

    let extra_action = protobuf::parse_from_bytes::<ExtraActionInfo>(&file_contents_bytes)
        .context("Failed to parse extra action protobuf")?;

    SPAWN_INFO.get(&extra_action).ok_or_else(|| anyhow!("SpawnInfo extension missing"))
}

/// Create an IndexedCompilation protobuf from the supplied arguments
///
/// * `source_files` - The names of the source files required by the build
///   target
/// * `arguments` - The arguments used to compile the target
/// * `build_output_path` - The output path of the build target
/// * `required_input` - The generated data for the CompilationUnit
///   `required_input` field
fn create_indexed_compilation(
    source_files: Vec<String>,
    arguments: Vec<String>,
    build_output_path: &str,
    required_input: Vec<CompilationUnit_FileInput>,
    mut unit_vname: VName,
) -> IndexedCompilation {
    let mut compilation_unit = CompilationUnit::new();

    unit_vname.set_language("rust".into());
    compilation_unit.set_v_name(unit_vname);
    compilation_unit.set_source_file(protobuf::RepeatedField::from_vec(source_files));
    compilation_unit.set_argument(protobuf::RepeatedField::from_vec(arguments));
    compilation_unit.set_required_input(protobuf::RepeatedField::from_vec(required_input));
    compilation_unit.set_output_key(build_output_path.to_string());

    let mut indexed_compilation = IndexedCompilation::new();
    indexed_compilation.set_unit(compilation_unit);
    indexed_compilation
}

/// Generate sha256 hex digest of a vector of bytes
fn sha256digest(bytes: &[u8]) -> String {
    let mut sha256 = Sha256::new();
    sha256.update(bytes);
    let bytes = sha256.finalize();
    hex::encode(bytes)
}

/// Add a file from a path to the kzip and the list of required inputs
///
/// * `file_path_string` - The string path of the file target
/// * `corpus` - The corpus to populate in the VName
/// * `zip_writer` - The ZipWriter to be written to
/// * `required_inputs` - The vector that the new CompilationUnit_FileInput will
///   be added to
fn kzip_add_required_input(
    file_path_string: &str,
    file_vname: VName,
    zip_writer: &mut ZipWriter<File>,
    required_inputs: &mut Vec<CompilationUnit_FileInput>,
) -> Result<()> {
    let mut source_file = File::open(file_path_string)
        .with_context(|| format!("Failed open file {:?}", file_path_string))?;
    let mut source_file_contents: Vec<u8> = Vec::new();
    source_file
        .read_to_end(&mut source_file_contents)
        .with_context(|| format!("Failed read file {:?}", file_path_string))?;
    let digest = sha256digest(&source_file_contents);
    kzip_add_file(format!("root/files/{}", digest), &source_file_contents, zip_writer)?;

    // Generate FileInput and add it to the list of required inputs
    let mut file_input = CompilationUnit_FileInput::new();
    file_input.set_v_name(file_vname);

    let mut file_info = FileInfo::new();
    file_info.set_path(file_path_string.to_string());
    file_info.set_digest(digest);
    file_input.set_info(file_info);

    required_inputs.push(file_input);
    Ok(())
}

/// Add a file to the kzip with the specified name and contents
///
/// * `file_name` - The new file's path inside the zip archive
/// * `file_bytes` - The byte contents of the new file
/// * `zip_writer` - The ZipWriter to be written to
fn kzip_add_file(
    file_name: String,
    file_bytes: &[u8],
    zip_writer: &mut ZipWriter<File>,
) -> Result<()> {
    zip_writer
        .start_file(&file_name, FileOptions::default())
        .with_context(|| format!("Failed create file in kzip: {:?}", file_name))?;
    zip_writer
        .write_all(file_bytes)
        .with_context(|| format!("Failed write file contents to kzip: {:?}", file_name))?;
    Ok(())
}

/// Generate the string path of the save_analysis file using the build target's
/// output path and the temporary base directory
fn analysis_path_string(build_output_path: &str, temp_dir_path: &Path) -> Result<String> {
    // Take the build output path and change the extension to .json
    let analysis_file_name = Path::new(build_output_path).with_extension("json");
    // Extract the file name from the path and convert to a string
    let analysis_file_str = analysis_file_name
        .file_name()
        .unwrap()
        .to_str()
        .ok_or_else(|| anyhow!("Failed to convert path to string"))?;

    // Join the temp_dir_path with "save-analysis/${analysis_file_str}" to get the
    // full path of the save_analysis JSON file
    let mut path = temp_dir_path.join("save-analysis").join(analysis_file_str);

    // The path should almost always exist. However, if the target name had
    // hyphens, then the final save_analysis file has underscores. We can't
    // always replace the hyphens with underscores because the save_analysis
    // files for libraries have hyphens in them.
    if !path.exists() {
        path = temp_dir_path.join("save-analysis").join(analysis_file_str.replace("-", "_"));
    }

    path.to_str()
        .ok_or_else(|| anyhow!("save_analysis file path is not valid UTF-8"))
        .map(|path_str| path_str.to_string())
}

fn create_vname(rules: &mut [VNameRule], path: &str, default_corpus: &str) -> VName {
    for rule in rules {
        if rule.matches(path) {
            return rule.produce_vname(path, default_corpus);
        }
    }
    // This should never happen but if we don't match at all just return an empty
    // vname with the corpus set
    let mut vname = VName::new();
    vname.set_corpus(default_corpus.to_string());
    vname
}
