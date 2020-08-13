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

#![feature(rustc_private)]

//! A Fuchsia OS compilation extractor.
//!
//! This is a purpose-built compilation extractor to be used on the [Fuchsia
//! OS][fxos] source code.  Please refer to the accompanying `README.md` file
//! for usage details.
//!
//! [fxos]: https://fuchsia.dev

use {
    analysis_rust_proto::*, // CompilationUnit, IndexedCompilation
    anyhow::{Context, Result},
    clap::clap_app,
    crypto::digest::Digest,
    crypto::sha2,
    fuchsia_extractor_lib::kzip,
    lazy_static::lazy_static,
    regex::Regex,
    rls_data,
    serde_json,
    std::fs,
    std::path::Path,
    std::path::PathBuf,
};

/// By convention, the source corpus name is "".
const SOURCE_CORPUS_NAME: &'static str = "";
const RLIBS_CORPUS_NAME: &'static str = "rlibs";

/// Reads the entire directory pointed at by `dir`.  The returned result
/// contains only names of regular files found.  The names must match `pattern`.
/// If `recursive` is set, any directories found will be recursed into.
fn read_dir(dir: &Path, recursive: bool, pattern: &Regex) -> Result<Vec<PathBuf>> {
    let mut files: Vec<PathBuf> = vec![];
    let contents = fs::read_dir(&dir)
        .with_context(|| format!("read_dir: while reading directory: {:?}", &dir))?;
    for f in contents {
        let f = f?;
        let file_type = f.file_type()?;
        if file_type.is_dir() {
            if !recursive {
                continue;
            }
            let recurse = read_dir(&f.path(), recursive, pattern).with_context(|| {
                format!("read_dir: while reading contents of directory: {:?}", f)
            })?;
            files.extend(recurse);
        } else {
            let path = f.path();
            let path_str = &path.to_string_lossy().to_string();
            if pattern.is_match(&path_str) && path_str != "" {
                files.push(path);
            }
        }
    }
    Ok(files)
}

lazy_static! {
    /// Matches all filenames ending with ".json".
    static ref MATCH_JSON: Regex = Regex::new(r".*\.json$").unwrap();
    /// Matches all filenames ending with ".rs".
    static ref MATCH_RUST: Regex = Regex::new(r".*\.rs$").unwrap();
}

/// Reads the file names of all files in the given directory.
fn read_save_analysis_dir(dir: Option<&str>) -> Result<Vec<PathBuf>> {
    match dir {
        None => Ok(vec![]),
        Some(ref d) => {
            let path = PathBuf::from(d);
            let files = read_dir(&path, false, &MATCH_JSON)
                .with_context(|| format!("while reading analysis dir: {:?}", &path));
            files
        }
    }
}

/// Generates a unique filename in `output_dir` for the kzip archive.
fn get_unique_filename(file: &PathBuf, output_dir: &PathBuf) -> Result<PathBuf> {
    let mut hasher = sha2::Sha256::new();
    let file_str = file.to_str().expect("should be convertible to string");

    hasher.input_str(file_str);
    let filename = format!("{}.rs.kzip", hasher.result_str());

    let mut output_filename = output_dir.clone();
    output_filename.push(filename);
    Ok(output_filename)
}

/// Canonicalizes the appearances of "." and ".." in the given path.  The effect
/// is similar to [std::fs::canonicalize], except no symlink resolution is
/// applied.  For this canonicalization to make sense, the supplied path must be
/// absolute.
fn partial_canonicalize_path(path: &PathBuf) -> Result<PathBuf> {
    if !path.is_absolute() {
        return Err(anyhow::anyhow!(
            "partial_canonicalize_path: path must be absolute to begin with: {:?}",
            path
        ));
    }
    let mut retained_components = vec![];
    for component in path.components() {
        match component {
            std::path::Component::ParentDir => {
                if retained_components.len() == 0 {
                    return Err(anyhow::anyhow!(
                        "partial_canoncalize_path: can not cd to .. beyond root in {:?}",
                        path
                    ));
                }
                retained_components.pop();
            }
            std::path::Component::CurDir => { /* Do nothing */ }
            _ => retained_components.push(component.as_os_str()),
        }
    }
    let result: PathBuf = retained_components.iter().collect();
    Ok(result)
}

/// Creates a VName for the given file.  The file name is rebased so that it
/// contains no references to parent, but has its path name relative to the
/// based dir of the specific corpus root.  The base_dir is expected to be one
/// of the ancestor directories of `file_name`, or an error will be reported.
/// `base_dir` does not need to be canonicalized, i.e. may contain `..`.
fn make_vname(
    file_name: &PathBuf,
    corpus: &str,
    root: &str,
    base_dir: &PathBuf,
    language: &str,
) -> Result<VName> {
    if !base_dir.is_absolute() {
        return Err(anyhow::anyhow!(
            "make_vname: base_dir must be absolute dir, but is: {:?}",
            base_dir
        ));
    }
    let rel_path = partial_canonicalize_path(&base_dir.join(file_name)).with_context(|| {
        format!(
            "make_vname: vname error while rebasing:\n\t{:?}\n\tinto\n\t{:?}",
            file_name, base_dir
        )
    })?;
    let base_dir = partial_canonicalize_path(&base_dir)?;
    let rel_path = make_relative_to(&rel_path, &base_dir)?;
    let mut vname = VName::new();
    vname.set_language(language.to_string());
    vname.set_corpus(corpus.to_string());
    vname.set_root(root.to_string());
    let rel_path_str = rel_path
        .to_str()
        .ok_or(anyhow::anyhow!("make_vname: could not convert to UTF-8: {:?}", &rel_path))?;
    if let Some(_) = rel_path_str.find("../") {
        return Err(anyhow::anyhow!(
            concat!(concat!(
                "make_vname: base_dir:\n\t{:?}\n\tis expected to be ",
                "an ancestor of file_name:\n\t{:?}\n\t",
                "but it is not.  This is contrary to kythe spec. We got:\n\t{:?}"
            )),
            base_dir,
            file_name,
            rel_path_str
        ));
    }
    vname.set_path(rel_path_str.to_string());
    Ok(vname)
}

/// Creates a CompilationUnit_FileInput from the supplied parts.  The creation
/// may fail if the file path is not UTF-8 clean. `file_name` is the relative
/// path to add to file input, `base_dir` is the directory where compilation
/// ran.
fn make_file_input(
    vname: VName,
    file_name: &PathBuf,
    base_dir: &PathBuf,
    digest: &str,
) -> Result<CompilationUnit_FileInput> {
    let file_name = base_dir.join(&file_name);
    let file_name = make_relative_to(&file_name, &base_dir)?;
    let mut file_info = FileInfo::new();
    file_info.set_path(
        file_name
            .to_str()
            .ok_or(anyhow::anyhow!("make_file_input: could not convert to UTF-8: {:?}", file_name))?
            .to_string(),
    );
    file_info.set_digest(digest.to_string());

    let mut result = CompilationUnit_FileInput::new();
    result.set_info(file_info);
    result.set_v_name(vname);
    Ok(result)
}

lazy_static! {
    /// A regular expression that matches the full path of a Rust rlib in
    /// the rustc compile arguments.  The argument would be like:
    ///     --extern keyword=dir1/dir2/libname.rlib
    static ref MATCH_RLIBS: Regex = Regex::new(r"(\w)+=(.*\.rlib)$").unwrap();
}

/// Examines a set of rustc arguments and extracts the "rlib" files from it.
fn extract_rlibs(arguments: &[impl AsRef<str>]) -> Vec<String> {
    let result: Vec<String> = arguments
        .iter()
        .filter(|e| MATCH_RLIBS.is_match(e.as_ref()))
        .map(|e| {
            let caps = MATCH_RLIBS.captures(e.as_ref()).unwrap();
            caps.get(2).unwrap().as_str().to_string()
        })
        .collect();
    result
}

/// Gets the crate name from the prelude data.
fn get_crate_name(prelude: &rls_data::CratePreludeData) -> String {
    let crate_id = &prelude.crate_id;
    crate_id.name.clone()
}

/// Gets the path to the crate root directory from the prelude.
fn get_crate_directory(prelude: &rls_data::CratePreludeData) -> PathBuf {
    let root_path = PathBuf::from(&prelude.crate_root);
    root_path.parent().unwrap_or(&PathBuf::from("")).to_path_buf()
}

/// Populates a single input into the Writer.
///
/// `path` is the (possibly relative) file path to the file to add.
/// `corpus_name` is mostly, "fuchsia".
/// `root` is a corpus root, for when a corpus has multiple roots.
/// `base_dir` is the base dir of the corpus root.  For example, a "src" corpus
/// is based in `$OUT_DIR/../../`, while the "gen" corpus may be based in
/// `$OUT_DIR/gen`.  `compilation_base_dir` is the directory where the
/// compilation ran.
fn add_input(
    archive: &mut kzip::Writer,
    corpus_name: &str,
    path: &PathBuf,
    root: &str,
    base_dir: &PathBuf,
    compilation_base_dir: &PathBuf,
    required_inputs: &mut Vec<String>,
    file_inputs: &mut Vec<CompilationUnit_FileInput>,
) -> Result<()> {
    // Ensure that we're targeting the path relative to base dir.
    let path = base_dir.join(&path);
    let path_from_compilation_dir = make_relative_to(&path, &compilation_base_dir)?;

    let content =
        fs::read(&path).with_context(|| format!("add_input: while trying to read: {:?}", &path))?;
    let digest = archive
        .write_file(&content)
        .with_context(|| format!("add_input: while writing content for: {:?}", &path))?;
    required_inputs.push(path_from_compilation_dir.to_string_lossy().to_string());
    file_inputs.push(make_file_input(
        make_vname(&path, corpus_name, root, &base_dir, "rust")?,
        &path,
        &base_dir,
        &digest,
    )?);
    Ok(())
}

/// Returns the name of the output produced by this save analysis rule.
fn get_compilation_output_name(compilation: &rls_data::CompilationOptions) -> PathBuf {
    compilation.output.clone()
}

/// Process one save-analysis file.
///
/// Generates a kzip archive with its compilation unit as result.
fn process_file(
    file: &PathBuf,
    output_dir: &PathBuf,
    analysis: &rls_data::Analysis,
    options: &Options,
) -> Result<PathBuf> {
    let file_relative_to_input = make_relative_to(&file, &options.base_dir)?;
    let kzip_filename = get_unique_filename(&file_relative_to_input, output_dir)?;

    if Path::new(&kzip_filename).exists() {
        return Err(anyhow::anyhow!(
            concat!(
                "process_file: archive already exists when processing file",
                ":\n\t{:?}, skipping archive:\n\t{:?}"
            ),
            &file,
            &kzip_filename
        ));
    }

    let mut archive =
        kzip::Writer::try_new(&kzip_filename, kzip::Encoding::Proto).with_context(|| {
            format!(
                "process_file: while creating archive:\n\t{:?}\nfrom file: \n\t{:?}",
                &kzip_filename, file
            )
        })?;

    let mut compilation_unit = CompilationUnit::new();

    let mut file_inputs: Vec<CompilationUnit_FileInput> = vec![];

    // Add the JSON file to the archive.
    let save_analysis_contents = fs::read(&file).with_context(|| {
        format!("process_file: while reading save analysis for storage: {:?}", &file)
    })?;
    let save_analysis_digest = archive
        .write_file(&save_analysis_contents)
        .with_context(|| format!("while saving save analysis for storage: {:?}", &file))?;
    file_inputs.push(make_file_input(
        make_vname(file, &options.corpus_name, "save-analysis", &options.base_dir, "rust")?,
        file,
        &options.base_dir,
        &save_analysis_digest,
    )?);

    // Add all arguments.
    let compilation = analysis
        .compilation
        .as_ref()
        .ok_or(anyhow::anyhow!("process_file: analysis JSON file has no compilation section"))?;
    let arguments: Vec<String> = compilation.arguments.clone();

    let prelude = analysis
        .prelude
        .as_ref()
        .ok_or(anyhow::anyhow!("process_file: analysis JSON file has no prelude section"))?;
    let crate_name = get_crate_name(&prelude);

    let mut required_inputs: Vec<String> = vec![];

    // Add each Rust file under the crate root directory into the kzip.
    let crate_root_directory = get_crate_directory(&prelude);
    let crate_root_directory = &options.base_dir.join(crate_root_directory);
    let rust_files = read_dir(&crate_root_directory, true, &MATCH_RUST).with_context(|| {
        format!("process_file: while reading crate root: {:?}", &crate_root_directory)
    })?;

    // The assumption is that the directory is 2 levels above base_dir, which
    // is true today in Fuchsia.
    let src_base_dir = &options.base_dir.join("../../");
    for src_path in rust_files {
        add_input(
            &mut archive,
            &options.corpus_name,
            &src_path,
            SOURCE_CORPUS_NAME,
            &src_base_dir,
            &options.base_dir,
            &mut required_inputs,
            &mut file_inputs,
        )
        .with_context(|| format!("process_file: while adding Rust file: {:?}", &src_path))?;
    }

    // For each rlib file under the directory, add it.
    let rlibs = extract_rlibs(&arguments);
    for rlib in rlibs {
        let rlib = PathBuf::from(rlib);
        add_input(
            &mut archive,
            &options.corpus_name,
            &rlib,
            RLIBS_CORPUS_NAME,
            &options.base_dir,
            &options.base_dir,
            &mut required_inputs,
            &mut file_inputs,
        )
        .with_context(|| format!("process_file: while adding rlib file: {:?}", &rlib))?;
    }

    // Build the master vname.
    let mut vname = VName::new();
    vname.set_corpus(options.corpus_name.clone());
    vname.set_language(options.language_name.clone());
    vname.set_signature(crate_name.clone());
    compilation_unit.set_v_name(vname);
    compilation_unit.set_argument(protobuf::RepeatedField::from_vec(arguments));
    compilation_unit.set_required_input(protobuf::RepeatedField::from_vec(file_inputs));
    compilation_unit.set_source_file(protobuf::RepeatedField::from_vec(required_inputs));
    let compilation_output_path = get_compilation_output_name(&compilation);
    compilation_unit.set_output_key(compilation_output_path.to_string_lossy().to_string());

    let abs_base_dir = fs::canonicalize(&options.base_dir).with_context(|| {
        format!("process_file: while trying to find absolute path of {:?}", &options.base_dir)
    })?;
    compilation_unit.set_working_directory(abs_base_dir.to_string_lossy().to_string());

    let mut index = IndexedCompilation_Index::new();
    index.set_revisions(protobuf::RepeatedField::from_vec(options.revisions.clone()));

    let mut indexed_compilation = IndexedCompilation::new();
    indexed_compilation.set_unit(compilation_unit);
    indexed_compilation.set_index(index);
    archive.write_unit(&indexed_compilation).with_context(|| {
        format!("process_file: while writing compilation unit for crate: {:?}", &crate_name)
    })?;
    let output_filename = output_dir.join(kzip_filename);
    Ok(output_filename)
}

/// Allows us to attach error context to a single file in case it is malformed,
/// so that we can safely skip its further processing without tanking the entire
/// extractor process.
///
/// For some reason the large save-analysis files that the compiler produces are
/// malformed in certain cases, so for the time being we want the ability to
/// skip over them safely.
fn lenient_process_file(file_name: &PathBuf, options: &Options) -> Result<PathBuf> {
    let file = fs::File::open(&file_name)
        .with_context(|| format!("process_files: while opening file: {:?}", &file_name))?;
    let analysis: rls_data::Analysis = serde_json::from_reader(file).with_context(|| {
        format!(
            "lenient_process_files: while parsing save-analysis JSON from file: {:?}",
            &file_name
        )
    })?;
    process_file(&file_name, &options.output_dir, &analysis, options).with_context(|| {
        format!("lenient_process_files: while processing file:\n\t{:?}", &file_name)
    })
}

/// Processes each save-analysis file in turn, extracting useful information
/// from it.
fn process_files(files: &[PathBuf], options: &Options) -> Result<Vec<PathBuf>> {
    let mut kzips: Vec<PathBuf> = vec![];
    for file_name in files {
        if !options.quiet {
            println!("\nprocess_files: processing {:?}", &file_name);
        }
        let kzip = lenient_process_file(&file_name, &options);
        match kzip {
            Err(ref e) => eprintln!("process_files: found error: {:?}", e),
            Ok(pb) => {
                if !options.quiet {
                    println!("process_files: `- made archive:\n\t{:?}", pb);
                }
                kzips.push(pb);
            }
        }
        if !options.quiet {
            println!("\n");
        }
    }
    Ok(kzips)
}

/// Makes an absolute `path` relative to its `parent`.  For example,
fn make_relative_to(path: &impl AsRef<Path>, parent: &impl AsRef<Path>) -> Result<PathBuf> {
    let mut src_components = path.as_ref().components().peekable();
    let mut dest_components = parent.as_ref().components().peekable();

    // Start from the canonical paths of source and destination.  Remove the maximal
    // shared prefix. Then assemble the final path from (1) the remaining path
    // components of dest, replaced by "..", and then remaining path components
    // of source.
    loop {
        let src = src_components.peek();
        let dest = dest_components.peek();
        match (src, dest) {
            (Some(s), Some(d)) => {
                if s != d {
                    break;
                }
                src_components.next();
                dest_components.next();
            }
            _ => {
                break;
            }
        }
    }
    let mut result = PathBuf::new();
    let _ = dest_components.map(|_| result.push("..")).collect::<()>();
    let _ = src_components.map(|c| result.push(&c)).collect::<()>();
    Ok(result)
}

/// The shared general settings for the compilation extractor.
struct Options {
    /// The name of this corpus, e.g. "fuchsia".
    corpus_name: String,
    /// The programming language this data is for, e.g. "rust"
    language_name: String,
    /// The directory in which compilation ran.
    base_dir: PathBuf,
    /// The directory to which to write the kzip file outputs.
    output_dir: PathBuf,
    /// The comma-separated revisions string, used to fill out the indexed
    /// compilation protocol buffer field called `index`.
    revisions: Vec<String>,
    /// If set, no non-error log messages are printed.
    quiet: bool,
}

fn main() -> Result<()> {
    // We'd rather use a crate that parses the program arguments directly into the
    // `Options` struct.  However, for some weird reason our dependencies can
    // not be so aligned to get any such crate to compile, even including a
    // newer version of `clap`, which supports this kind of parsing.  So we
    // parse like this.
    let matches = clap_app!{
        fuchsia_extractor =>
            (about: "A Kythe compilation extractor binary specifically made for the Fuchsia repository.")
            (@arg BASE_DIR: --basedir +takes_value "The directory from which the build was made, default '.'")
            (@arg QUIET: --quiet "If set, no non-error messages are logged")
            (@arg INPUT_DIR: --inputdir +takes_value "The directory containing save analysis files")
            (@arg INPUT: --inputfiles +takes_value "A comma-separated list of specific save-analysis files to read")
            (@arg OUTPUT_DIR: --output +takes_value +required "(required) The directory to save output kzips into; the directory must exist")
            (@arg CORPUS: --corpus +takes_value "The corpus name to use (defaults to env value of KYTHE_CORPUS)")
            (@arg LANGUAGE: --language +takes_value "The language to use (defaults to 'rust')")
            (@arg REVISIONS: --revisions +takes_value "Comma-separated list of revisions for IndexedCompilation.index")
    }.get_matches();

    // Clap version that we use has no direct parsing to options, so we do it this
    // way.
    let files_from_dirs = read_save_analysis_dir(matches.value_of("INPUT_DIR"))
        .with_context(|| format!("while reading input directories"))?;
    let explicit_files = matches
        .value_of("INPUT")
        .unwrap_or("")
        .split(",")
        .map(|e| e.into())
        .collect::<Vec<PathBuf>>();
    let all_files: Vec<PathBuf> =
        files_from_dirs.into_iter().chain(explicit_files.into_iter()).collect();
    let output_dir: PathBuf = matches.value_of("OUTPUT_DIR").unwrap().into();
    let corpus_name = matches
        .value_of("CORPUS")
        .unwrap_or(&std::env::var("KYTHE_CORPUS").unwrap_or("fuchsia".into()))
        .to_string();
    let language_name = matches.value_of("LANGUAGE").unwrap_or("rust");
    let base_dir: PathBuf = matches.value_of("BASE_DIR").unwrap_or(".").into();
    let revisions = matches
        .value_of("REVISIONS")
        .unwrap_or("")
        .to_string()
        .split(",")
        .map(|e| e.into())
        .collect::<Vec<String>>();
    let quiet = matches.is_present("QUIET");

    let options = Options {
        corpus_name: corpus_name.to_string(),
        language_name: language_name.to_string(),
        base_dir,
        output_dir,
        revisions,
        quiet,
    };
    process_files(&all_files, &options).with_context(|| "while reading save-analysis files")?;
    Ok(())
}

#[cfg(test)]
mod testing {
    use {super::*, std::collections::HashSet, std::fs, std::io::Read};

    /// Rebases the given `relative_path`, such that it is relative to
    /// `rebase_dir`. For example, "./foo/bar/file.txt", relative to "./foo"
    /// is "bar/file.txt".  But, relative to "./baz" is
    /// "../foo/bar/file.txt". `rebase_dir` must be a directory.  Both paths
    /// must exist on the filesystem.
    fn rebase_path(
        relative_path: impl AsRef<Path>,
        rebase_dir: impl AsRef<Path>,
    ) -> Result<PathBuf> {
        let fullpath_source = fs::canonicalize(&relative_path)
            .with_context(|| format!("while canonicalizing: {:?}", &relative_path.as_ref()))?;
        let fullpath_dest_dir = fs::canonicalize(&rebase_dir)
            .with_context(|| format!("while canonicalizing: {:?}", &rebase_dir.as_ref()))?;

        make_relative_to(&fullpath_source, &fullpath_dest_dir).with_context(|| {
            format!(
                "rebase_path: while making relative path for: {:?} based on {:?}",
                &relative_path.as_ref(),
                &rebase_dir.as_ref()
            )
        })
    }

    /// Gets Bazel's sharded temporary directory.  This is better than using the
    /// tempdir package or a similar solution, since adding `--sandbox_debug`
    /// flag to bazel will *not* remove the temporary directory, which is hugely
    /// useful for debugging.  In addition, this will play nice with remote
    /// builds and sandboxes.
    fn get_bazel_temp_dir() -> PathBuf {
        let temp_dir = PathBuf::from(std::env::var("TEST_TMPDIR").expect("temp dir is available"));
        let temp_dir = std::fs::canonicalize(&temp_dir).unwrap();
        temp_dir
    }

    #[test]
    fn test_rebase_path() {
        let temp_dir = get_bazel_temp_dir();
        #[derive(Debug)]
        struct TestCase {
            source: PathBuf,
            dest: PathBuf,
            expected: PathBuf,
        }
        let tests = vec![
            TestCase {
                source: "bar/baz/file.txt".into(),
                dest: "bar".into(),
                expected: "baz/file.txt".into(),
            },
            TestCase {
                source: "bar/baz/file.txt".into(),
                dest: "bar/baz".into(),
                expected: "file.txt".into(),
            },
            TestCase {
                source: "bar/baz/file.txt".into(),
                dest: "foo/bar".into(),
                expected: "../../bar/baz/file.txt".into(),
            },
            TestCase {
                source: "bar/baz/../file.txt".into(),
                dest: "foo/bar".into(),
                expected: "../../bar/file.txt".into(),
            },
            TestCase {
                source: "bar/baz/../file.txt".into(),
                dest: "bar/baz/..".into(),
                expected: "file.txt".into(),
            },
        ];
        for test in tests {
            let src = temp_dir.join(&test.source);
            fs::create_dir_all(&src).expect("source dir created");
            let dest_dir = temp_dir.join(&test.dest);
            fs::create_dir_all(&dest_dir).expect("dest dir created");
            let actual = rebase_path(&src, &dest_dir)
                .expect(&format!("rebase_path fails in test: {:?}", &test));
            assert_eq!(actual, test.expected, "mismatch in test: {:?}", &test);
        }
    }

    #[test]
    fn test_make_file_input() {
        let temp_dir = get_bazel_temp_dir();
        let base_dir = temp_dir.join("src-root-dir");
        fs::create_dir_all(&base_dir).expect("base dir created");
        let save_analysis_dir = base_dir.join("save-analysis-dir");
        fs::create_dir_all(&save_analysis_dir).expect("save analysis dir created");
        let file_name = save_analysis_dir.join("save-analysis.json");
        {
            // The file must exist on the filesystem for filename canonicalization to
            // work.
            let _json_file = fs::File::create(&file_name).expect("json file created");
        }
        // The file names we expect to get in the output are all relative with
        // respect to the base dir where compilation ran.
        let file_name = make_relative_to(&file_name, &base_dir).unwrap();

        let vname =
            make_vname(&file_name, "fuchsia", "root", &base_dir, "rust").expect("got vname");
        let input = make_file_input(vname, &file_name, &base_dir, "digest").expect("got input");

        // It would have been better to build this test protobuf from text proto,
        // but it seems that rust does not support that.
        let mut expected_vname = VName::new();
        expected_vname.set_corpus("fuchsia".to_string());
        expected_vname.set_language("rust".to_string());
        expected_vname.set_root("root".to_string());
        expected_vname.set_path("save-analysis-dir/save-analysis.json".to_string());
        let mut expected_file_info = FileInfo::new();
        expected_file_info.set_path(file_name.to_string_lossy().to_string());
        expected_file_info.set_digest("digest".to_string());
        let mut expected = CompilationUnit_FileInput::new();
        expected.set_v_name(expected_vname);
        expected.set_info(expected_file_info);

        assert_eq!(expected, input);
    }

    #[test]
    fn text_extract_rlibs() {
        let args = vec![
            "--extern",
            "a=foo.rlib",
            "--extern",
            "b=dir1/dir2/foo-something.rlib",
            "--extern",
            "some-other-thing=bar.rlib",
            "some-other-thing=bar.rlibbib",
            "some-other-thing=barrlib",
        ];
        let result = extract_rlibs(&args);
        assert_eq!(vec!["foo.rlib", "dir1/dir2/foo-something.rlib", "bar.rlib",], result);
    }

    #[test]
    fn test_read_dir_recursive() {
        let temp_dir = get_bazel_temp_dir();
        let base_dir = temp_dir.join("src-root-dir");
        fs::create_dir_all(&base_dir).expect("base dir created");
        let base_dir = rebase_path(base_dir, temp_dir).expect("rebase is a success");
        let save_analysis_dir = base_dir.join("save-analysis-dir");
        fs::create_dir_all(&save_analysis_dir).expect("save analysis dir created");
        {
            // The file must exist on the filesystem for filename canonicalization to
            // work.
            let file_name = save_analysis_dir.join("save-analysis.json");
            let _ = fs::File::create(&file_name).expect("json file created");
        }
        {
            // The file must exist on the filesystem for filename canonicalization to
            // work.
            let file_name = save_analysis_dir.join("save-analysis.txt");
            let _ = fs::File::create(&file_name).expect("json file created");
        }
        {
            // The file must exist on the filesystem for filename canonicalization to
            // work.
            let file_name = base_dir.join("some-file.txt");
            let _ = fs::File::create(&file_name).expect("file created");
        }

        // Reading save analysis directory gives the save analysis files.
        let result: HashSet<String> =
            read_save_analysis_dir(Some(&save_analysis_dir.to_string_lossy().to_string()))
                .expect("read was a success")
                .iter()
                .map(|e| e.to_string_lossy().to_string())
                .collect();
        assert_eq!(
            vec!["src-root-dir/save-analysis-dir/save-analysis.json".into()]
                .into_iter()
                .collect::<HashSet<String>>(),
            result
        );

        // Recursing all directories for a nonexistent file pattern yields no
        // results.
        let result: Vec<String> = read_dir(&base_dir, true, &Regex::new(".ext$").unwrap())
            .expect("read was a success")
            .iter()
            .map(|e| e.to_string_lossy().to_string())
            .collect();
        let expected: Vec<&'static str> = vec![];
        assert_eq!(expected, result);

        // Recursing all directories yields only files, regardless of the directory.
        // Using HashSet so this is order independent.
        let result: HashSet<String> = read_dir(&base_dir, true, &Regex::new(".*").unwrap())
            .expect("read was a success")
            .iter()
            .map(|e| e.to_string_lossy().to_string())
            .collect();
        assert_eq!(
            vec![
                "src-root-dir/save-analysis-dir/save-analysis.json".into(),
                "src-root-dir/save-analysis-dir/save-analysis.txt".into(),
                "src-root-dir/some-file.txt".into(),
            ]
            .into_iter()
            .collect::<HashSet<String>>(),
            result
        );

        // Non-recursive search in a directory yields the files in that directory.
        let result: Vec<String> = read_dir(&base_dir, false, &Regex::new(".*txt$").unwrap())
            .expect("read was a success")
            .iter()
            .map(|e| e.to_string_lossy().to_string())
            .collect();
        assert_eq!(vec!["src-root-dir/some-file.txt",], result);

        let result: Vec<String> = read_dir(&base_dir, false, &Regex::new(".*json$").unwrap())
            .expect("read was a success")
            .iter()
            .map(|e| e.to_string_lossy().to_string())
            .collect();
        let expected: Vec<String> = vec![];
        assert_eq!(expected, result);
    }

    fn unzip_compilation_unit(zip_path: impl AsRef<Path>) -> IndexedCompilation {
        let file = fs::File::open(&zip_path.as_ref())
            .expect(&format!("could not open zip file: {:?}", &zip_path.as_ref()));
        let mut zip = zip::ZipArchive::new(file)
            .expect(&format!("could not create zip file handle: {:?}", &zip_path.as_ref()));
        for i in 0..zip.len() {
            let mut file = zip.by_index(i).unwrap();
            if !file.is_file() || !file.name().starts_with("root/pbunits/") {
                continue;
            }
            let mut buf: Vec<u8> = vec![];
            file.read_to_end(&mut buf).unwrap();
            let result: analysis::IndexedCompilation = protobuf::parse_from_bytes(&buf).unwrap();
            return result;
        }
        panic!(format!(
            "pbunits file not found in existing zip file: zip_path: {:?}",
            &zip_path.as_ref(),
        ));
    }

    #[test]
    fn run_one_analysis() {
        let temp_dir = get_bazel_temp_dir();
        let test_srcdir =
            PathBuf::from(std::env::var("TEST_SRCDIR").expect("data dir is available"));
        let data_dir = test_srcdir
            .join("io_kythe/kythe/rust/fuchsia_extractor/testdata")
            .join("test_dir_1/compilation-root");

        let options = Options {
            corpus_name: "fuchsia".into(),
            language_name: "rust".into(),
            base_dir: data_dir.join("out/terminal.x64"),
            output_dir: temp_dir.clone(),
            revisions: vec!["revision1".into()],
            quiet: true,
        };
        let all_files: Vec<PathBuf> =
            vec![data_dir.join("out/terminal.x64/save-analysis-temp/nom.json")];
        let zips = process_files(&all_files, &options).expect("processing is successful");
        let only_archive = zips.get(0).unwrap();
        let indexed_compilation = unzip_compilation_unit(&only_archive);

        let compilation_unit = indexed_compilation.get_unit();
        let cu_vname = compilation_unit.get_v_name();

        // The checks below are a bit tedious, but we probably need it to guard
        // kzip conformance.
        let expected_arguments = vec![
            "--extern",
            "lazy_static=x64-shared/obj/third_party/rust_crates/liblazy_static-cdf593bd3fb3d68f.rlib",
            "--extern",
            "memchr=x64-shared/obj/third_party/rust_crates/libmemchr-523a962cdfbcb111.rlib",
            "--extern",
            "regex=x64-shared/obj/third_party/rust_crates/libregex-be3eec66bf66867d.rlib",
        ];
        assert_eq!(expected_arguments, compilation_unit.get_argument());

        let required_input = compilation_unit.get_required_input().get(0).unwrap();
        let ru_vname = required_input.get_v_name();
        assert_eq!("fuchsia", ru_vname.get_corpus());
        assert_eq!("save-analysis", ru_vname.get_root());

        let ru_info = required_input.get_info();
        assert_eq!("save-analysis-temp/nom.json", ru_info.get_path());

        // TODO(filmil): Not sure if this is what is meant by "signature" here.
        // This is a crate's name, and crate names are globally unique in Rust's
        // crate namespace.
        assert_eq!("nom", cu_vname.get_signature());

        let ri_one_file = compilation_unit.get_required_input().get(1).unwrap();
        let ru_vname = ri_one_file.get_v_name();
        assert_eq!("fuchsia", ru_vname.get_corpus());
        assert_eq!(SOURCE_CORPUS_NAME, ru_vname.get_root());

        let ru_info = ri_one_file.get_info();
        assert_eq!(
            "third_party/rust_crates/vendor/nom/lib.rs",
            ru_info.get_path(),
            concat!(
                "Expected to be relative to the root of its respective corpus",
                " for example for ../../dir/file.txt it will be dir/file.txt"
            )
        );
        assert_eq!("nom", cu_vname.get_signature());
    }
}
