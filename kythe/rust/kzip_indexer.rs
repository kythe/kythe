//! kzip_indexer parses Rust code from a kzip file and produces Kythe graph
//! data.
//!
//! By default, it writes binary kythe.proto.Entry protos to stdout, with
//! each prefixed by a varint length.
//! For debugging, pass --output=human-readable to get text protos instead.

// Deps:
//   :indexer
//   crates.io:log
//   kythe:analysis_rust_proto
//   kythe:storage_rust_proto
//   protobuf:protobuf
//   crates.io:anyhow
//   crates.io:clap
//   crates.io:integer_encoding
//   crates.io:zip
//   rust_analyzer

use analysis_rust_proto::{analysis_result, CompilationUnit, IndexedCompilation};
use clap::{Parser, ValueEnum};
use integer_encoding::VarInt;
use protobuf::Serialize;
use std::{
    fs::File,
    io::{stdout, BufReader, Read, Write},
    path::{Path, PathBuf},
    str,
};
use storage_rust_proto::EntryView;
use zip::ZipArchive;

#[derive(Parser)]
struct Flags {
    /// Input archives of source code + configuration.
    /// These are produced from a `rust_library()` by the kythe extractor.
    kzip: Vec<PathBuf>,

    /// Path to the standard library sources, where {std, core} etc can be found.
    #[clap(long)]
    sysroot_src: Option<PathBuf>,

    /// Path to the directory containing proc macro libraries.
    /// This is a proc_macro_bundle, see proc_macros/BUILD
    #[clap(long)]
    proc_macros: Option<PathBuf>,

    #[clap(long, value_enum, default_value_t=OutputFormat::BinaryProto)]
    output: OutputFormat,

    #[clap(long)]
    abbreviate_vnames: bool,
}

fn main() -> anyhow::Result<()> {
    let flags = Flags::parse();
    // Sysroot path needs to be absolute!
    let sysroot_src = match flags.sysroot_src {
        None => None,
        Some(path) => Some(indexer::AbsPathBuf::assert_utf8(std::fs::canonicalize(path)?)),
    };

    for path in flags.kzip {
        log::info!("Processing {}", path.to_string_lossy());
        let mut kzip = Kzip::open(&path)?;
        for comp in kzip.compilation_paths() {
            log::info!("Indexing {}", &comp);
            let cu = kzip.compilation(&comp)?;
            let result = indexer::index(
                cu.as_view(),
                &mut |digest| kzip.source(digest),
                sysroot_src.as_deref(),
                flags.proc_macros.as_deref(),
                &mut |entry| flags.output.write(entry),
                flags.abbreviate_vnames,
            );

            if result.status() != analysis_result::Status::Complete {
                anyhow::bail!("Indexing failed: {:?}", result);
            }
        }
    }

    Ok(())
}

/// Our inputs are kzips, which are zip files with a certain layout:
/// root/pbunits/<digest> - a CompilationUnit proto, refers to inputs by digest
/// root/files/<digest>   - an input file
struct Kzip(ZipArchive<BufReader<File>>);
impl Kzip {
    fn open(path: &Path) -> anyhow::Result<Kzip> {
        Ok(Kzip(ZipArchive::new(BufReader::new(File::open(path)?))?))
    }
    /// Return the paths inside the archive to all CompilationUnits.
    fn compilation_paths(&self) -> Vec<String> {
        self.0
            .file_names()
            .filter(|i| i.starts_with("root/pbunits/"))
            .map(|s| s.to_owned())
            .collect()
    }
    /// Decode the CompilationUnit with the specified path.
    fn compilation(&mut self, id: &str) -> anyhow::Result<CompilationUnit> {
        Ok(IndexedCompilation::parse(&self.read_raw(id)?)?.unit().to_owned())
    }

    fn read_raw(&mut self, path: &str) -> anyhow::Result<Vec<u8>> {
        let mut data = Vec::new();
        self.0.by_name(path)?.read_to_end(&mut data)?;
        Ok(data)
    }

    /// Allow the indexer to read required source files from the Kzip.
    fn source(&mut self, digest: &indexer::FileDigest) -> anyhow::Result<Vec<u8>> {
        self.read_raw(&format!("root/files/{}", str::from_utf8(digest)?))
    }
}

/// One format for programmatic users like verifier_test, another for debugging.
#[derive(ValueEnum, Copy, Clone, Debug)]
enum OutputFormat {
    BinaryProto,
    HumanReadable,
    None,
}
impl OutputFormat {
    fn write(&self, entry: EntryView) {
        match self {
            // Kythe's conventional format: a stream of binary Entry protos,
            // each prefixed with its size as an unsigned varint.
            OutputFormat::BinaryProto => {
                (|| -> anyhow::Result<()> {
                    let bytes = entry.serialize()?;
                    stdout().write_all(&(bytes.len() as u32).encode_var_vec())?;
                    stdout().write_all(&bytes)?;
                    Ok(())
                })()
                .expect("can't write to stdout!");
            }

            // Debug strings have are what we can get in Rust, for now.
            OutputFormat::HumanReadable => println!("{:?}", entry),
            OutputFormat::None => {}
        }
    }
}
