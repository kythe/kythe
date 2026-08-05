//! `parser` runs rust-analyzer's parser over a Kythe `CompilationUnit`.
//!
//! Mostly it's responsible for setting up the inputs rust-analyzer expects:
//! - a virtual filesystem to read sources from
//! - a sysroot providing the standard library
//! - a project model listing available crates and where their sources are.
//!   (This is derived from rust-project.json that the extractor produced).

// Deps:
//   crates.io:log
//   kythe:analysis_rust_proto
//   kythe:storage_rust_proto
//   protobuf:protobuf
//   crates.io:anyhow
//   crates.io:serde
//   crates.io:serde_json
//   :proc_macros/proc_macros
//   crates.io:walkdir
//   rust_analyzer

use std::{
    collections::{HashMap, HashSet},
    path::Path,
};

use analysis_rust_proto::CompilationUnitView;
use anyhow::{bail, Context};
use protobuf::proto;
use rust_analyzer::{
    base_db::{CrateBuilder, CrateDisplayName, CrateOrigin, LangCrateOrigin},
    cfg::CfgDiff,
    hir::{sym, CfgAtom, ChangeWithProcMacros, Crate, HasAttrs, ProcMacrosBuilder, Symbol},
    hir_expand::proc_macro::{ProcMacro, ProcMacroLoadResult},
    ide::{AnalysisHost, CrateGraphBuilder, FileChange, RootDatabase},
    ide_db::FxHashMap,
    load_cargo::ProjectFolders,
    paths::Utf8Path,
    project_model::{CargoConfig, CfgOverrides, ProjectJson, ProjectJsonData, ProjectWorkspace},
    vfs::{AbsPath, AbsPathBuf, FileId, Vfs, VfsPath},
};
use serde::{Deserialize, Serialize};
use storage_rust_proto::VName;

/// rust-analyzer's model of a crate.
#[derive(Debug)]
pub struct Parse {
    // Provides access to syntax and semantic analysis of the code.
    // Use `Semantics::new(host.raw_database())`
    pub host: AnalysisHost,
    // The primary crate described by the CompilationUnit.
    pub krate: Crate,
    // Do we suspect this is a standard-library crate?
    pub special: HashMap<Crate, LangCrateOrigin>,
    // The files comprising the current crate only.
    pub sources: HashMap<FileId, Vec<u8>>,
    // VNames of files mapped in the VFS.
    pub vnames: HashMap<FileId, VName>,
}

/// Identifier for file content.
///
/// Kythe uses a content-addressed store (both in KCD and inside kzip).
/// CompilationUnits map VFS paths onto content digests, and we use those
/// digests to load the content. (This helps the common case where CUs overlap).
///
/// In practice they are hexadecimal strings, but we treat them as opaque bytes.
pub type FileDigest = [u8];

/// Feed the provided compilation unit into rust-analyzer's parser.
/// The returned analysis host exposes both code syntax and semantics.
pub fn parse(
    cu: CompilationUnitView,
    sysroot_src: Option<&AbsPath>,
    proc_macro_dir: Option<&Path>,
    read: &mut impl FnMut(&FileDigest) -> anyhow::Result<Vec<u8>>,
) -> anyhow::Result<Parse> {
    if cu.v_name().language() != "rust" {
        bail!("cannot parse language={}", cu.v_name().language());
    }

    log::info!("parse: preparing VFS");
    let mut vfs = VFSBuilder::default();
    let sources = vfs.add_inputs(cu, read)?;
    if let Some(sysroot_src) = sysroot_src {
        vfs.add_standard_library(cu.v_name().corpus(), sysroot_src)?;
    }
    let VFSBuilder { vfs, mut source_change, vnames } = vfs;

    log::info!("parse: loading project model");
    // kythe-rust-project.json is a legacy name, use it as a fallback.
    let Some(manifest) = ["rust-project.json", "kythe-rust-project.json"]
        .into_iter()
        .filter_map(|name| cu.required_input().iter().find(|i| i.info().path() == name))
        .next()
    else {
        bail!("no rust-project.json")
    };
    let manifest = parse_project_json(
        &read(manifest.info().digest().into())?,
        sysroot_src.map(AbsPath::as_str),
    )?;
    // Proc macro has a list of target => dylib, so needs to know what targets go with what sources.
    let root_to_target: HashMap<FileId, String> =
        HashMap::from_iter(manifest.crates().filter_map(|(_, c)| {
            let (id, _file_excluded) = vfs.file_id(&VfsPath::from(c.root_module.clone()))?;
            let target = c.build.as_ref()?.label.clone();
            Some((id, target))
        }));
    configure_project(manifest, &vfs, &mut source_change)?;
    let target_to_macros = proc_macro_dir.map(proc_macros::load).transpose()?;
    let proc_macros = target_to_macros
        .map(|p| proc_macros(&p, source_change.crate_graph.as_ref().unwrap(), &root_to_target));

    log::info!("parse: semantic analysis");
    let mut host = AnalysisHost::new(/* lru_capacity is default */ None);
    host.apply_change(ChangeWithProcMacros { source_change, proc_macros });
    let db = host.raw_database();
    let krate = Crate::all(db)
        .into_iter()
        .find(|c| sources.contains_key(&c.root_file(db)))
        .context("main crate")?;
    let special = Crate::all(db)
        .into_iter()
        .filter_map(|c| detect_special_crate(db, c).map(|origin| (c, origin)))
        .collect();

    log::info!("parse: done");
    Ok(Parse { krate, special, sources, host, vnames })
}

/// Helper for populating rust-analyzer's virtual file system with source code.
/// This includes our target crate, its dependencies, and the standard library.
///
/// rust-analyzer splits responsibility between the VFS, which gives files
/// identity, and FileChange which populates their content.
/// This split doesn't do anything useful for us; we never update file contents.
#[derive(Default)]
struct VFSBuilder {
    vfs: Vfs,
    source_change: FileChange,
    vnames: HashMap<FileId, VName>,
}
impl VFSBuilder {
    /// Add a single file to the VFS.
    fn add(&mut self, path: AbsPathBuf, content: Vec<u8>, vname: VName) -> FileId {
        log::debug!("  {} - {} bytes", path, content.len());
        let text = String::from_utf8_lossy(&content).into_owned();
        let path = VfsPath::from(path);
        self.vfs.set_file_contents(path.clone(), Some(content));
        let (id, _file_excluded) = self.vfs.file_id(&path).expect("file missing after add");
        self.source_change.change_file(id, Some(text));
        self.vnames.insert(id, vname);
        id
    }

    /// Adds the compilation unit's required inputs to the VFS.
    /// Returns the main crate's sources.
    fn add_inputs(
        &mut self,
        cu: CompilationUnitView,
        read: &mut impl FnMut(&[u8]) -> anyhow::Result<Vec<u8>>,
    ) -> anyhow::Result<HashMap<FileId, Vec<u8>>> {
        let source_files = cu.source_file().into_iter().collect::<HashSet<_>>();
        let mut source_file_content = HashMap::new();
        let root = AbsPath::assert(Utf8Path::new(SRC_PATH));
        log::info!("Adding sources as {} from CU", root);
        for input in cu.required_input() {
            let bytes = read(input.info().digest().into())?;
            let source_copy =
                if source_files.contains(input.info().path()) { Some(bytes.clone()) } else { None };
            let id = self.add(
                root.join(Utf8Path::new(input.info().path().to_str()?)),
                bytes,
                input.v_name().to_owned(),
            );
            if let Some(bytes) = source_copy {
                source_file_content.insert(id, bytes);
            }
        }
        Ok(source_file_content)
    }

    /// Adds the standard library sources at the given path to the VFS.
    /// They are always mounted under the same path as on the real FS, because
    /// rust-analyzer's sysroot support is not VFS-clean.
    fn add_standard_library(
        &mut self,
        corpus: &protobuf::ProtoStr,
        sysroot_src: &AbsPath,
    ) -> anyhow::Result<()> {
        log::info!("Adding sysroot_src as {} from {}", sysroot_src, sysroot_src);
        for file in walkdir::WalkDir::new(sysroot_src)
            .follow_links(true)
            .into_iter()
            .filter_map(Result::ok)
            .filter(|e| e.file_type().is_file())
        {
            let Ok(bytes) = std::fs::read(file.path()) else { continue };
            let relative = file.path().strip_prefix(sysroot_src).unwrap();
            let vname = proto!(VName { corpus: corpus, path: relative.to_string_lossy() });
            self.add(
                sysroot_src.join(Utf8Path::from_path(relative).expect("utf-8 path")),
                bytes,
                vname,
            );
        }
        Ok(())
    }
}

/// Parse rust-project.json content into the form rust-analyzer expects.
/// We need to override the standard library path to where it's actually
/// mounted.
fn parse_project_json(bytes: &[u8], sysroot_src: Option<&str>) -> anyhow::Result<ProjectJson> {
    #[derive(Deserialize, Serialize)]
    struct PartialProjectJSON {
        sysroot_src: Option<String>,

        #[serde(flatten)]
        rest: HashMap<String, serde_json::Value>,
    }

    let mut p: PartialProjectJSON =
        serde_json::from_slice(bytes).context("parsing rust-project.json")?;
    p.sysroot_src = sysroot_src.map(str::to_string);
    let converted: ProjectJsonData =
        serde_json::from_slice(&serde_json::to_vec(&p)?).context("re-parsing rust-project.json")?;
    Ok(ProjectJson::new(None, AbsPath::assert(Utf8Path::new(SRC_PATH)), converted))
}

/// Turn the rust-project.json description into the model of crates and sources.
fn build_workspace(
    manifest: ProjectJson,
    vfs: &Vfs,
) -> anyhow::Result<(CrateGraphBuilder, ProjectFolders)> {
    let extra_env = FxHashMap::default();
    let cfg_overrides = CfgOverrides {
        global: CfgDiff::new(
            // Allow us to add errors inside tests, while hiding them from regular bazel builds.
            vec![CfgAtom::Flag(Symbol::intern("kythe_indexer_test"))],
            Vec::default(),
        ),
        ..Default::default()
    };

    let cargo_config = CargoConfig { extra_env, cfg_overrides, ..Default::default() };
    let mut workspace = ProjectWorkspace::load_inline(manifest, &cargo_config, &|_| ());
    // TODO(b/405358446): should we determine this from the per-crate "target" rather
    // than hard-coding? rust-analyzer obtains it by shelling out to rustc
    // --print target-spec-json.
    const DATA_LAYOUT: &str =
        "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128";
    workspace.target_layout = Ok(DATA_LAYOUT.into());
    if let Some(err) = workspace.sysroot.error() {
        bail!("bad sysroot: {}", err);
    }
    let (crate_graph, _macro_paths) = workspace.to_crate_graph(
        &mut |path: &AbsPath| {
            log::info!("{path}");
            vfs.file_id(&VfsPath::from(path.to_path_buf())).map(|(id, _file_excluded)| id)
        },
        &cargo_config.extra_env,
    );
    let project_folders = ProjectFolders::new(&[workspace], &[], None);

    Ok((crate_graph, project_folders))
}

/// Update the source_change to reflect the structure from rust-project.json.
fn configure_project(
    manifest: ProjectJson,
    vfs: &Vfs,
    source_change: &mut FileChange,
) -> anyhow::Result<()> {
    let (crate_graph, project_folders) = build_workspace(manifest, vfs)?;
    source_change.set_crate_graph(crate_graph);
    source_change.set_roots(project_folders.source_root_config.partition(vfs));
    Ok(())
}

/// The path that user code is mounted under in the VFS.
///
/// File paths in the CompilationUnit are relative to this directory.
const SRC_PATH: &str = "/src";

/// Check whether this crate is likely implementing part of the standard
/// library.
///
/// rust-analyzer will only treat a crate as stdlib if it was loaded from the
/// sysroot. This detection lets us correctly index the stdlib as regular
/// rust_librarys.
///
/// We consider the crate name and the presence of #![stable(feature = "...")].
fn detect_special_crate(db: &RootDatabase, krate: Crate) -> Option<LangCrateOrigin> {
    if let CrateOrigin::Lang(special) = krate.origin(db) {
        return Some(special);
    }

    let attrs = krate.root_module().attrs(db);

    // Support #![crate_name], mostly for use in our tests.
    let crate_name = Symbol::intern("crate_name");
    let attr_name = attrs.by_key(crate_name).string_value();
    let display_name = krate.display_name(db);
    let physical_name = display_name.as_ref().map(CrateDisplayName::canonical_name);
    let name = attr_name.or(physical_name)?;

    let stable = Symbol::intern("stable");
    let feature = attrs.by_key(stable).find_string_value_in_tt(sym::feature)?;
    match (name.as_str(), feature) {
        ("alloc", "alloc") => Some(LangCrateOrigin::Alloc),
        ("core", "core") => Some(LangCrateOrigin::Core),
        ("std", "rust1") => Some(LangCrateOrigin::Std),
        ("proc_macro", "proc_macro_lib") => Some(LangCrateOrigin::ProcMacro),
        _ => None,
    }
}

/// Match proc macro crates against loaded dylibs, based on bazel target name.
fn proc_macros(
    target_to_macros: &HashMap<String, Vec<ProcMacro>>,
    crates: &CrateGraphBuilder,
    root_to_target: &HashMap<FileId, String>,
) -> ProcMacrosBuilder {
    let find_macro = |krate: &CrateBuilder| -> ProcMacroLoadResult {
        let file = krate.basic.root_file_id;
        let target = root_to_target.get(&file).ok_or_else(|| {
            // The error boolean affects whether this is exposed as "warning" or "error" via LSP.
            (format!("missing build target {:?}", krate.extra.display_name), /*error=*/ true)
        })?;
        target_to_macros
            .get(target)
            .cloned()
            .ok_or_else(|| (format!("unsupported proc macro {target}"), /*error=*/ false))
    };
    crates
        .iter()
        .map(|id| (id, &crates[id]))
        .filter(|(_id, krate)| krate.basic.is_proc_macro)
        .map(|(id, krate)| (id, find_macro(krate)))
        .collect()
}

#[cfg(test)]
mod tests {
    use analysis_rust_proto::{compilation_unit, CompilationUnit, FileInfo};
    use anyhow::Context;
    use googletest::{
        expect_that, gtest,
        matchers::{anything as any, eq, ok, some, unordered_elements_are},
    };
    use protobuf::proto;
    use protobuf_gtest_matchers::proto_eq;
    use rust_analyzer::{
        hir::{DisplayTarget, ModuleDef, Name},
        vfs::AbsPathBuf,
    };
    use std::{collections::HashMap, path::PathBuf};
    use storage_rust_proto::VName;

    struct TestFiles(HashMap<&'static str, &'static str>);
    impl TestFiles {
        fn read(&self, digest: &[u8]) -> anyhow::Result<Vec<u8>> {
            self.0
                .get(&String::from_utf8_lossy(digest).strip_prefix("digest:").unwrap())
                .map(|s| s.as_bytes().to_vec())
                .context("not found")
        }
        fn file_inputs(&self) -> Vec<compilation_unit::FileInput> {
            self.0
                .keys()
                .map(|path| {
                    proto!(compilation_unit::FileInput {
                        info: FileInfo { path: *path, digest: format!("digest:{path}") },
                        v_name: VName { corpus: "g3", path: *path },
                    })
                })
                .collect()
        }
    }

    #[gtest]
    fn test_parse() -> anyhow::Result<()> {
        let files = TestFiles(HashMap::from([
            ("foo/lib.rs", "mod submod; pub fn foo() -> u8 { return bar::BAR + submod::BAZ; }"),
            ("foo/submod.rs", "const BAZ = 43; }"),
            ("bar/lib.rs", "const BAR : u8 = 42;"),
            (
                "rust-project.json",
                r#"{
                    "crates":[
                      {
                        "display_name":"bar",
                        "root_module":"bar/lib.rs",
                        "edition":"2018",
                        "deps":[],
                        "cfg":[],
                        "is_proc_macro":false
                      },
                      {
                        "display_name":"foo",
                        "root_module":"foo/lib.rs",
                        "edition":"2021",
                        "deps":[
                          {"crate":0,"name":"bar"}
                        ],
                        "cfg":[],
                        "is_proc_macro":false
                      }
                    ]
                  }"#,
            ),
        ]));
        let cu = proto!(CompilationUnit {
            v_name: VName { corpus: "g3", language: "rust" },
            source_file: ["foo/lib.rs", "foo/submod.rs"],
            required_input: files.file_inputs().into_iter(),
        });
        let parse = crate::parse(cu.as_view(), None, None, &mut |digest| files.read(digest))?;
        let db = parse.host.raw_database();

        // Check source files mapping.
        expect_that!(
            parse.sources,
            unordered_elements_are![
                (any(), eq(files.0["foo/lib.rs"].as_bytes())),
                (any(), eq(files.0["foo/submod.rs"].as_bytes())),
            ]
        );
        expect_that!(
            parse.vnames,
            unordered_elements_are![
                (any(), proto_eq(proto!(VName { corpus: "g3", path: "foo/lib.rs" }))),
                (any(), proto_eq(proto!(VName { corpus: "g3", path: "foo/submod.rs" }))),
                (any(), proto_eq(proto!(VName { corpus: "g3", path: "bar/lib.rs" }))),
                (any(), proto_eq(proto!(VName { corpus: "g3", path: "rust-project.json" }))),
            ]
        );

        // Check project structure.
        expect_that!(parse.krate.display_name(db).map(|n| n.to_string()), some(eq("foo")));
        expect_that!(
            parse.vnames.get(&parse.krate.root_file(db)),
            some(proto_eq(proto!(VName { corpus: "g3", path: "foo/lib.rs" })))
        );

        // Check parse results.
        let decl_names: Vec<String> = parse
            .krate
            .root_module()
            .declarations(db)
            .into_iter()
            .filter_map(|d| d.name(db).map(|n| n.as_str().to_owned()))
            .collect();
        expect_that!(decl_names, unordered_elements_are![eq("foo"), eq("submod")]);

        Ok(())
    }

    /// Test that we can load and use a standard library (here, just libcore).
    #[gtest]
    fn test_stdlib() -> anyhow::Result<()> {
        // Core exposes a constant in the prelude.
        const CORE_SOURCE: &str = r#"
          pub mod prelude {
            pub mod rust_2021 {
              pub const FROM_CORE : usize = 42;
            }
          }
        "#;
        // If rust-analyzer knows that X = 42 then the stdlib was successfully used.
        const USER_SOURCE: &str = r#"
          #![no_std]
          const X : usize = FROM_CORE;
        "#;

        let files = TestFiles(HashMap::from([
            ("foo/lib.rs", USER_SOURCE),
            (
                "rust-project.json",
                r#"{
                    "crates":[
                      {
                        "display_name":"foo",
                        "root_module":"foo/lib.rs",
                        "edition":"2021",
                        "deps":[],
                        "cfg":[],
                        "is_proc_macro":false,
                        "target":"x86_64-unknown-linux-gnu"
                      }
                    ]
                  }"#,
            ),
        ]));
        // Mock standard library must be on physical disk.
        let sysroot_src =
            AbsPathBuf::assert_utf8(PathBuf::from(std::env::var("TEST_TMPDIR")?).join("stdlib"));
        assert!(!std::fs::exists(&sysroot_src)?);
        std::fs::create_dir_all(&sysroot_src)?;
        std::fs::create_dir(sysroot_src.join("libcore"))?;
        std::fs::write(sysroot_src.join("libcore").join("lib.rs"), CORE_SOURCE)?;

        let cu = proto!(CompilationUnit {
            v_name: VName { corpus: "g3", language: "rust" },
            source_file: ["foo/lib.rs", "foo/submod.rs"],
            required_input: files.file_inputs().into_iter(),
        });
        let parse =
            crate::parse(cu.as_view(), Some(&sysroot_src), None, &mut |digest| files.read(digest))?;
        let db = parse.host.raw_database();

        // Check source files mapping.
        expect_that!(
            parse.vnames,
            unordered_elements_are![
                (any(), proto_eq(proto!(VName { corpus: "g3", path: "foo/lib.rs" }))),
                (any(), proto_eq(proto!(VName { corpus: "g3", path: "rust-project.json" }))),
                (any(), proto_eq(proto!(VName { corpus: "g3", path: "libcore/lib.rs" }))),
            ]
        );

        // Check parse results.
        match parse.krate.root_module().declarations(db).first() {
            Some(ModuleDef::Const(x)) => {
                expect_that!(x.name(db).as_ref().map(Name::as_str), some(eq("X")));
                let target = DisplayTarget::from_crate(db, parse.krate.into());
                expect_that!(x.eval(db).map(|c| c.render(db, target)), ok(eq("42")));
            }
            _ => anyhow::bail!("could not find constant X"),
        }

        Ok(())
    }
}
