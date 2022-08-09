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

mod save_analysis;

use analysis_rust_proto::*;
use anyhow::{anyhow, Context, Result};
use clap::Parser;
use extra_actions_base_rust_proto::*;
use glob::glob;
use kythe_rust_extractor::vname_util::VNameRule;
use protobuf::Message;
use sha2::{Digest, Sha256};
use std::fs::File;
use std::io::{Read, Write};
use std::path::{Path, PathBuf};
use std::{env, fs};
use tempdir::TempDir;
use zip::{write::FileOptions, ZipWriter};

#[derive(Parser)]
#[clap(author = "The Kythe Authors")]
#[clap(about = "Kythe Rust Extractor", long_about = None)]
#[clap(rename_all = "snake_case")]
struct Args {
    /// Path to the extra action file
    #[clap(long, value_parser, value_name = "FILE")]
    extra_action: PathBuf,

    /// Desired output path for the kzip
    #[clap(long, value_parser, value_name = "DIR")]
    output: PathBuf,

    /// Location of the vnames configuration file
    #[clap(long, value_parser, value_name = "FILE")]
    vnames_config: PathBuf,
}

fn main() -> Result<()> {
    let config = Args::parse();

    // Parse the VName configuration rules
    let mut vname_rules = VNameRule::parse_vname_rules(&config.vnames_config)?;

    // Retrieve the SpawnInfo from the extra action file
    let spawn_info = get_spawn_info(&config.extra_action)?;

    // Grab the environment variables from spawn info and set them in our current
    // environment
    let environment_variables: Vec<_> = spawn_info.get_variable().to_vec();
    let pwd = std::env::current_dir().context("Couldn't determine pwd")?;
    let mut out_dir_var: Option<String> = None;
    for variable in environment_variables {
        let original_value = variable.get_value();
        let value = original_value.replace("${pwd}", pwd.to_str().unwrap());
        let name = variable.get_name();
        std::env::set_var(name, value);
        if name == "OUT_DIR" {
            // We want the original value so that we can easily get file paths
            // relative to OUT_DIR later
            out_dir_var = Some(original_value.to_string());
        }
    }

    // Get paths for files present in $OUT_DIR
    let mut out_dir_inputs: Vec<String> = Vec::new();
    if let Some(out_dir) = out_dir_var {
        let absolute_out_dir = out_dir.replace("${pwd}", pwd.to_str().unwrap());
        let glob_pattern = format!("{}/**/*", absolute_out_dir);
        for path in glob(&glob_pattern).unwrap().flatten() {
            out_dir_inputs.push(path.display().to_string());
        }
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
    let kzip_file = File::create(&config.output)
        .with_context(|| format!("Failed to create kzip file at path {:?}", &config.output))?;
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
    let mut required_inputs: Vec<CompilationUnit_FileInput> = Vec::new();
    for file_path in &rust_source_files {
        // Generate the file vname based on the vname rules
        let file_vname: VName = create_vname(&mut vname_rules, file_path, &default_corpus);
        kzip_add_required_input(file_path, file_vname, &mut kzip, &mut required_inputs)?;
    }

    // Add files that were present in $OUT_DIR during compilation as required inputs
    let mut out_dir_rust_source_files = Vec::new();
    for path in out_dir_inputs {
        // We strip the current working directory so we can generate the proper VName,
        // as the resulting path will usually start with "bazel-out/".
        let relative_path = path.replace(&format!("{}/", pwd.to_str().unwrap()), "");
        let file_vname: VName = create_vname(&mut vname_rules, &relative_path, &default_corpus);
        // We path the absolute path when adding the file as a required input because
        // that is the path will show up in the save_analysis. We want it set as the
        // FileInfo path so it can be properly matched to its VName during indexing.
        kzip_add_required_input(&path, file_vname, &mut kzip, &mut required_inputs)?;
        // If this is a Rust file, add it to the vec
        if path.ends_with(".rs") {
            out_dir_rust_source_files.push(path);
        }
    }

    // Grab the build target's output path
    let build_output_path = spawn_info
        .get_output_file()
        .get(0)
        .ok_or_else(|| anyhow!("Failed to get output file from spawn info"))?;

    // Infer the crate name from the compiler args, falling back to the output file
    // name if necessary
    let crate_name = {
        let crate_name_matches: Vec<_> =
            build_target_arguments.iter().filter(|arg| arg.starts_with("--crate-name=")).collect();
        if !crate_name_matches.is_empty() {
            crate_name_matches.get(0).unwrap().replace("--crate-name=", "")
        } else {
            PathBuf::from(build_output_path)
                .file_name()
                .unwrap()
                .to_str()
                .ok_or_else(|| anyhow!("Failed to convert build output path file name to string"))?
                .to_string()
        }
    };

    // Add save analysis to kzip
    let save_analysis_path: String = analysis_path_string(&crate_name, tmp_dir.path())?;
    let save_analysis_vname: VName =
        create_vname(&mut vname_rules, &save_analysis_path, &default_corpus);
    kzip_add_required_input(
        &save_analysis_path,
        save_analysis_vname,
        &mut kzip,
        &mut required_inputs,
    )?;

    // Create the IndexedCompilation and add it to the kzip
    let main_source_path = &rust_source_files[0];
    let mut unit_vname = create_vname(&mut vname_rules, main_source_path, &default_corpus);
    if !default_corpus.is_empty() {
        unit_vname.set_corpus(default_corpus);
    }
    let mut all_source_files = Vec::new();
    all_source_files.extend(rust_source_files);
    all_source_files.extend(out_dir_rust_source_files);
    let indexed_compilation = create_indexed_compilation(
        all_source_files,
        build_target_arguments,
        build_output_path,
        required_inputs,
        unit_vname,
        env::current_dir()?
            .to_str()
            .ok_or_else(|| anyhow!("working directory is invalid UTF-8"))?,
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

    let extra_action: ExtraActionInfo = protobuf::Message::parse_from_bytes(&file_contents_bytes)
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
/// * `working_directory` - The working_directory field for the CompilationUnit
fn create_indexed_compilation(
    source_files: Vec<String>,
    arguments: Vec<String>,
    build_output_path: &str,
    required_input: Vec<CompilationUnit_FileInput>,
    mut unit_vname: VName,
    working_directory: &str,
) -> IndexedCompilation {
    let mut compilation_unit = CompilationUnit::new();

    unit_vname.set_language("rust".into());
    compilation_unit.set_v_name(unit_vname);
    compilation_unit.set_source_file(protobuf::RepeatedField::from_vec(source_files));
    compilation_unit.set_argument(protobuf::RepeatedField::from_vec(arguments));
    compilation_unit.set_required_input(protobuf::RepeatedField::from_vec(required_input));
    compilation_unit.set_output_key(build_output_path.to_string());
    compilation_unit.set_working_directory(working_directory.to_string());

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

/// Find the path of the save_analysis file using the build target's
/// crate name and the temporary base directory
fn analysis_path_string(crate_name: &str, temp_dir_path: &Path) -> Result<String> {
    let entries = fs::read_dir(temp_dir_path.join("save-analysis")).with_context(|| {
        format!("Failed to read the save_analysis temporary directory: {:?}", temp_dir_path)
    })?;

    for entry in entries {
        let entry = entry.with_context(|| "Failed to get information about directory entry")?;
        let metadata = entry.metadata().with_context(|| "Failed to get entry metadata")?;
        let path = entry.path();
        let path_string = path
            .to_str()
            .ok_or_else(|| anyhow!("save_analysis file path is not valid UTF-8"))
            .map(|path_str| path_str.to_string())?;
        if metadata.is_file() && path_string.contains(crate_name) && path_string.ends_with(".json")
        {
            return Ok(path_string);
        }
    }

    Err(anyhow!("Failed to find save-analysis file in {:?}", temp_dir_path))
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
