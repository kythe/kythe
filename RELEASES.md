# Release Notes

## [v0.0.30] - 2019-02-07

#### Features

*   add extract_kzip tool to release archive (#3454) ([3ab6111d](https://github.com/kythe/kythe/commit/3ab6111d222cb120a89155ecdc6687c06c0ed7e7))
*   pass-through build config in pipeline (#3444) ([59be834f](https://github.com/kythe/kythe/commit/59be834f3ed1600fc1dcbe5d3fbdafc27c3aa2bd))
*   add build configuration to anchors in serving data (#3440) ([e3c7fa18](https://github.com/kythe/kythe/commit/e3c7fa18eaee9315834a51a4c0f46678c9d98138))
*   optionally use a default VName if no rules match (#3428) ([c8db8ccf](https://github.com/kythe/kythe/commit/c8db8ccf2783d92e29af8df6eda8e4a5533e3f54))
*   add protobuf to Bazel indexing example (#3423) ([51d5867f](https://github.com/kythe/kythe/commit/51d5867f09146a1538ff808b19b5be3162a007b6))
*   add Typescript kzip extractor extra_action (#3411) ([d5a79f22](https://github.com/kythe/kythe/commit/d5a79f22edb45b83a004f7e047ac35757fcdefa7))
*   add protobuf indexer to release (#3358) ([ae537104](https://github.com/kythe/kythe/commit/ae537104b1cbe0fbb6911f534643d209c9c42ed6))
*   add kzip cli tool (#3354) ([275f8d91](https://github.com/kythe/kythe/commit/275f8d9102a6009d307d1dd6052916dd86cd97b9))
*   add ability for kzip_validator to validate based on -repo_url (#3343) ([04f07ead](https://github.com/kythe/kythe/commit/04f07ead07a2d6073f9c59e846117f096e45e91e))
*   add a simple kzip validator (#3336) ([c10f87f0](https://github.com/kythe/kythe/commit/c10f87f082aecfe90fc93930ccb9714da518a117))
*   add example for extracting multiple mavens out of guava in gcb (#3332) ([a423b401](https://github.com/kythe/kythe/commit/a423b401bd63655a892e3db9a2acb5e4748cba8d))
*   add docker image with a kythe install for use in google cloud build (#3318) ([6f2caac6](https://github.com/kythe/kythe/commit/6f2caac6ef0023ac05d2112b3dd31215b8a14f11))
*   support reading kzip files in kindex_tool (#3293) ([2be0c1f0](https://github.com/kythe/kythe/commit/2be0c1f0d1519d24903572b32c42de593eb80449))
*   add ReadConcurrency scan option to kzip (#3283) ([f9c2e64d](https://github.com/kythe/kythe/commit/f9c2e64dd3f047341230dfb4aa79c57bd403b227))
*   add visibility aliases and mark some protos/utils as public (#3278) ([f749ea98](https://github.com/kythe/kythe/commit/f749ea9804ad3dec0fd4b0357d1e8eca13f23541))
*   add C++ extraction/analysis to Bazel example (#3274) ([1ea13daf](https://github.com/kythe/kythe/commit/1ea13dafedc7f6cb243112ed7e24a7cf15091d39))
*   support emitting USRs for other kinds of declarations (#3268) ([4d705cf9](https://github.com/kythe/kythe/commit/4d705cf9b55fe00d286e159d63b7ca35d0885a52))
*   adding generator for kythe maven/gradle based on config proto (#3219) ([da78a8e0](https://github.com/kythe/kythe/commit/da78a8e07eb23e00b56b8d0cad67c245880f8281))
*   Support adding Clang USRs to the graph (#3226) ([15535c65](https://github.com/kythe/kythe/commit/15535c65f50b7a0e867d3aca9a5d71d1c302b8c9))
*   add proto for configuring kythe extraction on a repo (#3206) ([c23007af](https://github.com/kythe/kythe/commit/c23007af5680fe5075d37e05c7c44fc142d81dc3))
*   add Seek method to keyvalue.Iterator interface (#3211) ([753c91ae](https://github.com/kythe/kythe/commit/753c91ae536c0a803c7a10d7900a6c9af2c38fc4))
*   limit disksort memory by custom Size method (#3201) ([7919fcf1](https://github.com/kythe/kythe/commit/7919fcf1d5c6e182d3db16ed48c1caa4186e7ff8))
* **api:**  add DecorationsRequest filter for build config (#3449) ([2480a935](https://github.com/kythe/kythe/commit/2480a9357b755fb204d7847bcd42626263226f5a))
* **bazel_extractor:**  infer corpus from source files (#3413) ([b7388af4](https://github.com/kythe/kythe/commit/b7388af43599fb67300998e224c2a2158e5cc2f2))
* **build_details:**  add a build_config field to BuildDetails proto (#3303) ([ed5ce4d5](https://github.com/kythe/kythe/commit/ed5ce4d53c5714cb3f56d876842ef54c92eb1063))
* **columnar:**
  *  support build_config filtering of columnar decorations (#3450) ([810c9211](https://github.com/kythe/kythe/commit/810c9211df728b447e7d070002e9d8d2857eed0a))
  *  add definitions for related node xrefs (#3228) ([52a635eb](https://github.com/kythe/kythe/commit/52a635eb80ba2b31d8d6eb6b37e855b3fe49a0ae))
* **cxx:**  copy in utf8 line index utils (#3276) ([93e69d29](https://github.com/kythe/kythe/commit/93e69d29d6ccd801c7067907c3f0dd4b0c6eed8b))
* **cxx_indexer:**
  *  include build/config fact on anchors when present (#3437) ([96c7d6bc](https://github.com/kythe/kythe/commit/96c7d6bcbead16fa3a99ce03750e413a7b901d3c))
  *  Add support for member pointers and uses of them a… (#3258) ([a83e856d](https://github.com/kythe/kythe/commit/a83e856d789b375bfef5ee9dad27169c121d7cd6))
  *  read all compilations from a kzip file (#3232) ([3bd99ee7](https://github.com/kythe/kythe/commit/3bd99ee7c251298cfdcb3860ea16ca54e47b66c4))
* **entrystream:**  support aggregating entries into an EntrySet (#3363) ([e1b38f50](https://github.com/kythe/kythe/commit/e1b38f5084b1adc4719d594dee080e6c64084fe3))
* **filetree:**  structured entries in DirectoryReply (#3425) ([53e3a097](https://github.com/kythe/kythe/commit/53e3a0978b478eb716dd88ee70ae69f97f426bc7))
* **gotool_extractor:**
  *  canonicalize corpus names as package/repo roots (#3377) ([f2630cbc](https://github.com/kythe/kythe/commit/f2630cbcf51e3b301bbbf3c3e716f4365dd159e6))
  *  add Docker image for Go extractor (#3340) ([f1ef34b5](https://github.com/kythe/kythe/commit/f1ef34b5e227e4807ae65173ff0e1e325971fe9e))
  *  use Go tool to extract package data (#3338) ([a97f5c0e](https://github.com/kythe/kythe/commit/a97f5c0e5928d7ac4cc2dcf7856731bd111e3f50))
* **java_indexer:**  emit implicit anchors for default constructors (#3317) ([90d1abfe](https://github.com/kythe/kythe/commit/90d1abfe6b45cf6aa1050f03dbfdd5fe7d4a7d8b))
* **llvm:**  update LLVM to r353272 (#3447) ([229c72dd](https://github.com/kythe/kythe/commit/229c72ddd2fd7cb9c4819b6c356e86c6d88bba80))
* **objc_indexer:**
  *  support marked source for @property (#3320) ([b79d49bd](https://github.com/kythe/kythe/commit/b79d49bd6805901f77f8bab58cd108ec046e6c47))
  *  marked source for category methods (#3311) ([414bdf2d](https://github.com/kythe/kythe/commit/414bdf2de1443b7f60becddd99c0619bbf9506f1))
* **post-processing:**
  *  add diagnostics to file decorations (#3277) ([0cd5dfca](https://github.com/kythe/kythe/commit/0cd5dfcad100d40275690ff3d1f2e4147c0212ba))
  *  add support for Riegeli input files (#3223) ([4035f931](https://github.com/kythe/kythe/commit/4035f931711eadd18d81fba7332e6eee83065d4f))
  *  emit columnar callers (#3220) ([e1fe01a6](https://github.com/kythe/kythe/commit/e1fe01a63cae0b9d732323c6a2af0785522eb45f))
  *  allow `write_tables` to compact output LevelDB (#3215) ([2895c1c7](https://github.com/kythe/kythe/commit/2895c1c7d14783edd8dd40d7d87f983a730ae437))
* **sample-web-ui:**
  *  add link to related node definitions (#3227) ([854ab489](https://github.com/kythe/kythe/commit/854ab489cc3f09ddd82d71ba19e9d86f86090a19))
  *  display callers in xrefs panel (#3221) ([5c563dc7](https://github.com/kythe/kythe/commit/5c563dc744f2b7e58511ecfd9ba2ef6190486653))
* **xrefs:**  handle columnar decl/def CrossReferences (#3204) ([eec113df](https://github.com/kythe/kythe/commit/eec113df604ae70c75ecbd0f941930e45b91131e))

#### Bug Fixes

*   remove an obsolete test case (#3442) ([d14e93cc](https://github.com/kythe/kythe/commit/d14e93cccf96c90e01f3044d05f290d9482056b3))
*   minor changes to gcp/config gen using examples (#3432) ([cc74e161](https://github.com/kythe/kythe/commit/cc74e16187a2b0e4dda5b6abef60e4ebbc2ed507))
*   add an ordering-agnostic yaml comparator in testutil (#3430) ([abde75cb](https://github.com/kythe/kythe/commit/abde75cba42f925038e8c089e3d4bc69d3eda4fd))
*   remove unnecessary call to default_javac_opts (#3417) ([997b7da8](https://github.com/kythe/kythe/commit/997b7da8083aafc30cd640e0e45d79ffe72466c5))
*   support pluginManagement in mvn.pom preprocessor (#3409) ([a8e8cc41](https://github.com/kythe/kythe/commit/a8e8cc419f98a879a029751ba639b810bfaf5e6b))
*   removing extraneous bits from guava exmaple yaml (#3404) ([41e42037](https://github.com/kythe/kythe/commit/41e42037feada206e3146cb9b24b7522cd7ae179))
*   adding example config for guava-android, distinct from guava (#3403) ([37251c2d](https://github.com/kythe/kythe/commit/37251c2d40f47df04e6b6802571ad0bc06680a3f))
*   use static_cast to avoid sign-compare warning (#3399) ([f2dbc60f](https://github.com/kythe/kythe/commit/f2dbc60f9c53f6a9eb28e7dd25e578c0a58625d6))
*   add short hint in tools/modules/update on how force rebuild (#3396) ([f4cbc604](https://github.com/kythe/kythe/commit/f4cbc604bb63c091e334156650a4515b8c2eafa9))
*   add documentation on the limitations of `build //...` (#3393) ([2f1c42b9](https://github.com/kythe/kythe/commit/2f1c42b9e65a40cddb71a3157c25f2cebe7a09c9))
*   fix a typo in a comment (#3390) ([41480d6b](https://github.com/kythe/kythe/commit/41480d6b6ceeedb06406a35ae6aff49962251af9))
*   use kzip tool instead of zipmerge for gcp extraction (#3375) ([31ef9eed](https://github.com/kythe/kythe/commit/31ef9eed6d896898a99ac01d2b6392913091f753))
*   downstream clang-tidy fix (#3372) ([6c54d48c](https://github.com/kythe/kythe/commit/6c54d48ce947fb5526c50e3264bdeb35fea3a685))
*   Prepare for a breaking change in clang (#3368) ([0b88cb2d](https://github.com/kythe/kythe/commit/0b88cb2d9ddc25ec7313873f2c191ffb8d58f4c7))
*   don't use a string pointer when we don't have to (#3367) ([5747281a](https://github.com/kythe/kythe/commit/5747281adf7d4fc36bfcbda33c7414cd0a5f7a30))
*   add docs on extracting non-github repo (#3366) ([186a1889](https://github.com/kythe/kythe/commit/186a18896edbdbadf4c89c10d137a205a0e9fb24))
*   replace zipmerge with 'kzip merge' (#3356) ([30b83a30](https://github.com/kythe/kythe/commit/30b83a306e2d7713b48847183e0b366872fe337e))
*   extract guava tests in GCB example as well (#3341) ([1191e069](https://github.com/kythe/kythe/commit/1191e06949ab4437268453a29e8986157c72f9b3))
*   check for sentinel versions of llvm/clang in check.sh (#3326) ([fa9d06a8](https://github.com/kythe/kythe/commit/fa9d06a8678df3fb1c41deb6a2935f90928b7a2e), closes [#3324](https://github.com/kythe/kythe/issues/3324))
*   include wget in the list of packages to install on macOS (#3325) ([e1e6fa69](https://github.com/kythe/kythe/commit/e1e6fa69cdbcd2f2feb09b59cac807cefcbec846))
*   change tools/modules/update.sh to use an archive instead of clone (#3308) ([434c0b0f](https://github.com/kythe/kythe/commit/434c0b0f25ee645e7b95c13adf497a80dc72f754))
*   The link to the new macos docs was broken (#3316) ([3ecd75c3](https://github.com/kythe/kythe/commit/3ecd75c3295f3fc516b9b443bd97eb36d4a02b5b))
*   Cleanup typos in objc test (#3310) ([8733094d](https://github.com/kythe/kythe/commit/8733094de09599d6eb72f91e6a1633aaccbd1371))
*   add -fsanitize= to the unsupported args list (#3289) ([091819bd](https://github.com/kythe/kythe/commit/091819bdbf98251e61f4cbc12d0a140897b7f1d8))
*   add BUILD visibility for targets required for proto indexer (#3299) ([91da8b42](https://github.com/kythe/kythe/commit/91da8b42354cd3b6818be5a9bf4389fd144ff6e5))
*   don't crash on empty function/enum bodies (#3281) ([f06d335d](https://github.com/kythe/kythe/commit/f06d335dffca49c17d6d2ff543c9cc4d9fea943c))
*   make workaround source compatible with JDK 11 (#3275) ([7284ac2c](https://github.com/kythe/kythe/commit/7284ac2cb2610b1fa07e3ca4081a9dddbee0f74c))
*   spelling (#3257) ([2a47e7b2](https://github.com/kythe/kythe/commit/2a47e7b21ccede520a0464c62baa2e0ac35d8cb6))
*   forgot test files from earlier commit, adding (#3252) ([e46a0552](https://github.com/kythe/kythe/commit/e46a0552344e5e605d94dc0efb5f0af1ce0062d4))
*   allow git hooks to not exist (#3239) ([441bb333](https://github.com/kythe/kythe/commit/441bb333d03262f861d46c1fc90cb76e1c4fa414))
*   remove spammy log line (#3238) ([c65fa3ca](https://github.com/kythe/kythe/commit/c65fa3ca906a7e108a90c8832ff2cd4073993126))
*   uploading error file first for better error proagation (#3235) ([411bf1b2](https://github.com/kythe/kythe/commit/411bf1b2cecf78b58c8e93d8b7c734f6a81df550))
*   truncate USR SHA1, not USR; kUsr -> kClangUsr (#3230) ([3a907384](https://github.com/kythe/kythe/commit/3a90738462fce018ea63ba26e23f4fbdb4002c54))
*   specify revision to diff against when using arc (#3229) ([6ef46fd0](https://github.com/kythe/kythe/commit/6ef46fd058899ce21f0d6970d9593174347d42f3))
*   resolve clang warnings (#3213) ([458bca0e](https://github.com/kythe/kythe/commit/458bca0e838b87e11a39474392de792b8fb6640f))
*   modernize Bazel end-to-end example scripts (#3209) ([c3373a27](https://github.com/kythe/kythe/commit/c3373a271f2fc99245cc81f2307266ca90c52ac9), closes [#3089](https://github.com/kythe/kythe/issues/3089))
* **bazel extractor:**  print file counts after handling source args (#3418) ([ff26749e](https://github.com/kythe/kythe/commit/ff26749ed0f5e937b04f2bb962fced4f49b9b6cb))
* **cxx:**
  *  use static_cast to avoid sign-compare warning (#3349) ([b1847b00](https://github.com/kythe/kythe/commit/b1847b00496523d90c8dc3faf1df4faa67c64254))
  *  add visibility for :status library for use in proto indexer (#3321) ([6d4fcaf0](https://github.com/kythe/kythe/commit/6d4fcaf0e10d13d74f46b6c5e7a243d83fd158be))
* **cxx frontend:**  set CompilationUnit.source_file for unpacked input (#3254) ([35706fb1](https://github.com/kythe/kythe/commit/35706fb1d1a526c4b3bc07f35e9e2206a921c20d))
* **cxx_common:**
  *  add enumerator for kDocUri fact (#3378) ([4b8925e0](https://github.com/kythe/kythe/commit/4b8925e0d8ffc5373a2f03c3489e8a9d8c7de1e9))
  *  Add a dependency for strict dependency checking (#3269) ([5a5a230e](https://github.com/kythe/kythe/commit/5a5a230ec48059a9a3203b29b7526fd183752ea1))
* **cxx_extractor:**  segfault when given nonexistent file (#3234) ([6c0fef7a](https://github.com/kythe/kythe/commit/6c0fef7ae8223707f5f546130f22272a1dfc6b7d))
* **docker_gotool_extractor:**  use stable output path, if possible (#3401) ([10da00f8](https://github.com/kythe/kythe/commit/10da00f8d8f2a04b16d4581982b09c4d8117b8c8))
* **docs:**
  *  clean up LLVM docs for #3441 (#3443) ([e0271fc6](https://github.com/kythe/kythe/commit/e0271fc6b0782ecdf3e23e4a522acf49cc4ed859))
  *  a few documentation updates (#3262) ([5f518734](https://github.com/kythe/kythe/commit/5f5187348c218b948caf46b7a74fc528ccf0630e))
  *  fixed a few broken links (#3236) ([25fdda35](https://github.com/kythe/kythe/commit/25fdda35d0081b82855450f517301fee6788bf0a))
* **examples:**  fix a missing dependecy from a docker rule (#3420) ([4f5e9730](https://github.com/kythe/kythe/commit/4f5e9730130cfc799b87e7e7a7b3d13d7b60a3a4))
* **go:**  use GoPackageInfo to resolve input import paths (#3408) ([cb77bf40](https://github.com/kythe/kythe/commit/cb77bf40cd8775a7dfc0788a436dca5293ea3247))
* **gotool_extractor:**
  *  fix file paths with --canonicalize_package_corpus (#3400) ([56fd0af1](https://github.com/kythe/kythe/commit/56fd0af1362c16c374a0ff3cdeff8959f9824470))
  *  mark dep as missing if it cannot be built (#3380) ([5f1a9627](https://github.com/kythe/kythe/commit/5f1a96272d7d4b3a691e4ab248e21d0fba560002))
  *  properly continue on package analysis errors (#3376) ([59276a9e](https://github.com/kythe/kythe/commit/59276a9e3c30e3408ecfe857ca301b34d8c51894))
  *  pass-through full build context (#3342) ([d55b14b5](https://github.com/kythe/kythe/commit/d55b14b5abffa2fbf59e9809e28aa69183c6002e))
  *  when no global corpus is given, use package's corpus for each file (#3290) ([6bc18f57](https://github.com/kythe/kythe/commit/6bc18f5769e6330da3a35afa515fcdcd19cefde2))
* **java_common:**  allow analyzers to throw InterruptedExceptions (#3330) ([01617d9c](https://github.com/kythe/kythe/commit/01617d9cc0753550964c57af57c64644a0808798))
* **java_indexer:**
  *  ensure static import refs are accessible (#3305) ([c453a319](https://github.com/kythe/kythe/commit/c453a319b76da635c7915c8376d0121118c2d8bd))
  *  ensure static import refs are static (#3294) ([0f80fe48](https://github.com/kythe/kythe/commit/0f80fe4816e6e3235113277a616beb9b80701b8c))
  *  do not close System.out after first analysis (#3199) ([e2af342f](https://github.com/kythe/kythe/commit/e2af342f151c090a679f2c6a2d914ba9c724d36e))
* **json_proto:**  prepare for upstream case-sensitivity changes (#3347) ([322571c8](https://github.com/kythe/kythe/commit/322571c80166d04317d466f89865687e1c04d572))
* **jvm_indexer:**  prepare code using ASM for Java 11 (#3214) ([94810956](https://github.com/kythe/kythe/commit/948109561829caa91c32378c580ea51192b88b7e))
* **post-processing:**  remove anchors from edge/xref relations (#3198) ([b81ef3af](https://github.com/kythe/kythe/commit/b81ef3afc21ea292c2357faae972ba8cb6b15fc4))
* **rfc2967:**  clarify rules around config-dependent generated files (#3355) ([d93af92d](https://github.com/kythe/kythe/commit/d93af92d0e8ce76fce6a5271473ecdccf5d7ad20))
* **sh_test:**  use "set -e" instead of adding "-e" to shebang (#3295) ([656082c6](https://github.com/kythe/kythe/commit/656082c602c50c520452c6dc2f7e9c2c8d563889))
* **testdata.proto:**  explicitly set java_package and dep on jsr250 (#3387) ([de4d9665](https://github.com/kythe/kythe/commit/de4d9665d0c79daf24d818007d2ff3a2780e5ee2))
* **web_api:**  properly marshal protos with jsonpb (#3424) ([358b4060](https://github.com/kythe/kythe/commit/358b4060eaddb32189fd36073d2ac9f28261a707))

## [v0.0.29] - 2018-10-29

### Added
 - KZip support has been added to all core extractors/indexers
 - `cxx_indexer`: define a deprecated tag and use it for C++ (#2982)
 - `java_indexer`: analyze default annotation values (#3004)

### Changed
 - `java_indexer`: do not include Symbol modifiers in hashes (#3139)
 - `javac_extractor`: migrate javac_extractor to use ambient langtools (#3093)
 - `verifier`: recover file VNames using file content; reorder singleton checking (#3166)

### Deprecated
 - Index packs and `.kindex` files have been deprecated in favor of `.kzip` files

### Fixed
 - `go_indexer`: mark result parameters as part of the TYPE in MarkedSource (#3021)
 - `java_indexer`: avoid NPE with erroneous compilations missing identifier Symbols (#3007)
 - `java_indexer`: guard against null inferred lambda types (#3132)
 - `java_indexer`: support JDK 11 API change (#3149)
 - `javac_extractor`: ignore JDK 9 modules as well as JDK 8 jars (#2889)
 - `javac_extractor`: pass through -processorpath; don't delete gensrcdir early (#3063)

## [v0.0.28] - 2018-07-18

### Added
 - `write_tables`: `--experimental_beam_pipeline` runs the post-processor as an
   [Apache Beam] pipeline
 - Go: support extracting/analyzing packages with `vendor/` dirs
 - `go_indexer`: add `--continue` flag to log errors without halting analysis
 - `viewindex`: add --file flag to print a single file's contents
 - `entrystream`: support for reading/writing [Riegeli] record files
 - First release of the [ExploreService] APIs

### Deprecated
 - `entrystream`: `--read_json`/`--write_json` are marked to be replaced by
   `--read_format=json`/`--write_format=json`

### Fixed
 - Java indexer: search for ClassSymbol in all Java 9 modules
 - Java/JVM indexers: obtain JVM (a.k.a. "external") names in a principled way

[Apache Beam]: https://beam.apache.org/
[ExploreService]: https://github.com/kythe/kythe/blob/master/kythe/proto/explore.proto
[Riegeli]: https://github.com/google/riegeli

## [v0.0.27] - 2018-07-01

Due to the period of time between this release and v0.0.26, many relevant
changes and fixes may not appear in the following list.  For a complete list of
changes, please check the commit logs:
https://github.com/kythe/kythe/compare/v0.0.26...v0.0.27

### Added
 - First release of Go indexer and Go extractors.
 - Objective C is now supported in the C++ indexer.
 - identifier.proto: adds a new `IdentifierService` API.
 - Runtime plugins can be added to the Java indexer.

### Changed
 - Remove gRPC server and client code.
 - Schema:
   - Anchors no longer have `childof` edges to their parent `file`.
   - Anchor nodes must share their `path`, `root`, and `corpus` `VName`
     components with their parent `file`.
  - Format strings have been replaced by the `MarkedSource` protobuf message.
 - C++ analysis:
   - Included files are referenced by the `ref/file` edge.
   - `code` facts (containing `MarkedSource` protos) are emitted.
   - Better support for C++11, 14, and 17.
   - Limited CUDA support.
   - Improvements to indexing and rendering of documentation.
   - Added a plugin for indexing proto fields in certain string literals.
   - Type ranges for builtin structural types are no longer destructured (`T*`
     is now `[T]*`, not `[[T]*]`).
   - Decoration of builtin types can be controlled with
     `--emit_anchors_on_builtins`.
   - Namespaces are never `define`d, only `ref`d.
   - Old-style `name` nodes removed.
   - The extractor now captures more build state, enabling support for
     `__has_include`
   - `anchor`s involved in `completes` edges now contain their targets mixed
     with their signatures, making each completion relationship unique.
   - Support for indexing uses of `make_unique` et al as direct references to
     the relevant constructor.
   - Template instantiations can be aliased together with
     `--experimental_alias_template_instantiations`, which significantly
     decreases output size at the expense of lower fidelity.
 - Java analysis:
   - `name` nodes are now defined as JVM binary names.
   - `diagnostic` nodes are emitted on errors.
   - `code` facts (`MarkedSource` protos) are emitted.
   - Add callgraph edges for constructors.
   - Non-member classes are now `childof` their enclosing method.
   - Local variables are now `childof` their enclosing method.
   - Blame calls in static initializers on the enclosing class.
   - Emit references for all matching members of a static import.
   - Reference `abs` nodes for generic classes instead of their `record` nodes.
   - Emit data for annotation arguments.
   - Emit package relations from `package-info.java` files.
 - Protocol Buffers:
   - analysis.proto: add revision and build ID to AnalysisRequest.
   - analysis.proto: add AnalysisResult summary to AnalysisOutput.
   - analysis.proto: remove `revision` from CompilationUnit.
   - graph.proto: new location of the GraphService API
   - xref.proto: remove `DecorationsReply.Reference.source_ticket`.
   - xref.proto: add `Diagnostic` messages to `DecorationsReply`.
   - xref.proto: replace `Location.Point` pairs with `common.Span`.
   - xref.proto: by default, elide snippets from xrefs/decor replies.
   - xref.proto: replace `Printable` formats with `MarkedSource`.
   - xref.proto: allow filtering related nodes in the xrefs reply.
   - xref.proto: optionally return documentation children.
   - xref.proto: return target definitions with overrides.

## [v0.0.26] - 2016-11-11

### Changed
 - Nodes and Edges API calls have been moved from XRefService to GraphService.

## [v0.0.25] - 2016-10-28

### Changed
 - Replace google.golang.org/cloud dependencies with cloud.google.com/go
 - Update required version of Go from 1.6 to 1.7

## [v0.0.24] - 2016-08-16

### Fixed
 - write_tables now tolerates nodes with no facts. Previously it could sometimes crash if this occurred.

## [v0.0.23] - 2016-07-28

### Changed
 - CrossReferences API: hide signature generation behind feature flag
 - Java indexer: emit `ref/imports` anchors for imported symbols

### Added
 - Java indexer: emit basic `format` facts

## [v0.0.22] - 2016-07-20

### Changed
 - Schema: `callable` nodes and `callableas` edges have been removed.
 - `xrefs.CrossReferences`: change Anchors in the reply to RelatedAnchors
 - Removed search API

## [v0.0.21] - 2016-05-12

### Changed
 - xrefs service: replace most repeated fields with maps
 - xrefs service: add `ordinal` field to each EdgeSet edge
 - `xrefs.CrossReferences`: group declarations/definitions for incomplete nodes
 - C++ indexer: `--flush_after_each_entry` now defaults to `true`

### Added
 - `xrefs.Decorations`: add experimental `target_definitions` switch
 - kythe tool: add `--graphviz` output flag to `edges` subcommand
 - kythe tool: add `--target_definitions` switch to `decor` subcommand

### Fixed
  `write_tables`: correctly handle nodes with missing facts
 - Javac extractor: add processors registered in META-INF/services
 - javac-wrapper.sh: prepend bootclasspath jar to use packaged javac tools

## [v0.0.20] - 2016-03-03

### Fixed
 - Java indexer: reduce redundant AST traversals causing large slowdowns

## [v0.0.19] - 2016-02-26

### Changed
 - C++ extractor: `KYTHE_ROOT_DIRECTORY` no longer changes the working
   directory during extraction, but does still change the root for path
   normalization.
 - `http_server`: ensure the given `--serving_table` exists (do not create, if missing)
 - Java indexer: fixes/tests for interfaces, which now have `extends` edges
 - `kythe` tool: display subkinds for related nodes in xrefs subcommand

### Added
 - `entrystream`: add `--unique` flag
 - `write_tables`: add `--entries` flag

## [v0.0.18] - 2016-02-11

### Changed
 - C++ indexer: `--ignore_unimplemented` now defaults to `true`
 - Java indexer: emit single anchor for package in qualified identifiers

### Added
 - Java indexer: add callgraph edges
 - Java indexer: add Java 8 member reference support

## [v0.0.17] - 2015-12-16

### Added
 - `write_tables`: produce serving data for xrefs.CrossReferences method
 - `write_tables`: add flags to tweak performance
     - `--compress_shards`: determines whether intermediate data written to disk
       should be compressed
     - `--max_shard_size`: maximum number of elements (edges, decoration
       fragments, etc.) to keep in-memory before flushing an intermediary data
       shard to disk
     - `--shard_io_buffer`: size of the reading/writing buffers for the
       intermediary data shards

## [v0.0.16] - 2015-12-08

### Changed
 - Denormalize the serving table format
 - xrefs.Decorations: only return Reference targets in DecorationsReply.Nodes
 - Use proto3 JSON mapping for web requests: https://developers.google.com/protocol-buffers/docs/proto3#json
 - Java indexer: report error when indexing from compilation's source root
 - Consistently use corpus root relative paths in filetree API
 - Java, C++ indexer: ensure file node VNames to be schema compliant
 - Schema: File nodes should no longer have the `language` VName field set

### Added
 - Java indexer: emit (possibly multi-line) snippets over entire surrounding statement
 - Java indexer: emit class node for static imports

### Fixed
 - Java extractor: correctly parse @file arguments using javac CommandLine parser
 - Java extractor: correctly parse/load -processor classes
 - xrefs.Edges: correctly return empty page_token on last page (when filtering by edge kinds)

## [v0.0.15] - 2015-10-13

### Changed
 - Java 8 is required for the Java extractor/indexer

### Fixed
 - `write_tables`: don't crash when given a node without any edges
 - Java extractor: ensure output directory exists before writing kindex

## [v0.0.14] - 2015-10-08

### Fixed
 - Bazel Java extractor: filter out Bazel-specific flags
 - Java extractor/indexer: filter all unsupported options before yielding to the compiler

## [v0.0.13] - 2015-09-17

### Added
 - Java indexer: add `ref/doc` anchors for simple class references in JavaDoc
 - Java indexer: emit JavaDoc comments more consistently; emit enum documentation

## [v0.0.12] - 2015-09-10

### Changed
 - C++ indexer: rename `/kythe/edge/defines` to `/kythe/edge/defines/binding`
 - Java extractor: change failure to warning on detection of non-java sources
 - Java indexer: `defines` anchors span an entire class/method/var definition (instead of
                 just their identifier; see below for `defines/binding` anchors)
 - Add public protocol buffer API/message definitions

### Added
 - Java indexer: `ref` anchors span import packages
 - Java indexer: `defines/binding` anchors span a definition's identifier (identical
                  behavior to previous `defines` anchors)
 - `http_server`: add `--http_allow_origin` flag that adds the `Access-Control-Allow-Origin` header to each HTTP response

## [v0.0.11] - 2015-09-01

### Added
 - Java indexer: name node support for array types, builtins, files, and generics

### Fixed
 - Java indexer: stop an exception from being thrown when a line contains multiple comments

## [v0.0.10] - 2015-08-31

### Added
 - `http_server`: support TLS HTTP2 server interface
 - Java indexer: broader `name` node coverage
 - Java indexer: add anchors for method/field/class definition comments
 - `write_table`: add `--max_edge_page_size` flag to control the sizes of each
                  PagedEdgeSet and EdgePage written to the output table

### Fixed
 - `entrystream`: prevent panic when given `--entrysets` flag

## [v0.0.9] - 2015-08-25

### Changed
 - xrefs.Decorations: nodes will not be populated unless given a fact filter
 - xrefs.Decorations: each reference has its associated anchor start/end byte offsets
 - Schema: loosened restrictions on VNames to permit hashing

### Added
 - dedup_stream: add `--cache_size` flag to limit memory usage
 - C++ indexer: hash VNames whenever permitted to reduce output size

### Fixed
 - write_tables: avoid deadlock in case of errors

## [v0.0.8] - 2015-07-27

### Added
 - Java extractor: add JavaDetails to each CompilationUnit
 - Release the indexer verifier tool (see http://www.kythe.io/docs/kythe-verifier.html)

### Fixed
 - write_tables: ensure that all edges are scanned for FileDecorations
 - kythe refs command: normalize locations within dirty buffer, if given one

## [v0.0.7] - 2015-07-16

### Changed
 - Dependencies: updated minimum LLVM revision. Run tools/modules/update.sh.
 - C++ indexer: index definitions and references to member variables.
 - kwazthis: replace `--ignore_local_repo` behavior with `--local_repo=NONE`

### Added
 - kwazthis: if found, automatically send local file as `--dirty_buffer`
 - kwazthis: return `/kythe/edge/typed` target ticket for each node

## [v0.0.6] - 2015-07-09

### Added
 - kwazthis: allow `--line` and `--column` info in place of a byte `--offset`
 - kwazthis: the `--api` flag can now handle a local path to a serving table

### Fixed
 - Java indexer: don't generate anchors for implicit constructors

## [v0.0.5] - 2015-07-01

### Added
 - Bazel `extra_action` extractors for C++ and Java
 - Implementation of DecorationsRequest.dirty_buffer in xrefs serving table

## [v0.0.4] - 2015-07-24

### Changed
 - `kythe` tool: merge `--serving_table` flag into `--api` flag

### Fixed
 - Allow empty requests in `http_server`'s `/corpusRoots` handler
 - Java extractor: correctly handle symlinks in KYTHE_ROOT_DIRECTORY

## [v0.0.3] - 2015-07-16

### Changed
 - Go binaries no longer require shared libraries for libsnappy or libleveldb
 - kythe tool: `--log_requests` global flag
 - Java indexer: `--print_statistics` flag

## [v0.0.2] - 2015-06-05

### Changed
 - optimized binaries
 - more useful CLI `--help` messages
 - remove sqlite3 GraphStore support
 - kwazthis: list known definition locations for each node
 - Java indexer: emit actual nodes for JDK classes

## [v0.0.1] - 2015-05-20

Initial release

[Unreleased] https://github.com/kythe/kythe/compare/v0.0.30...HEAD
[v0.0.30] https://github.com/kythe/kythe/compare/v0.0.29...v0.0.30
[v0.0.29] https://github.com/kythe/kythe/compare/v0.0.28...v0.0.29
[v0.0.28]: https://github.com/kythe/kythe/compare/v0.0.27...v0.0.28
[v0.0.27]: https://github.com/kythe/kythe/compare/v0.0.26...v0.0.27
[v0.0.26]: https://github.com/kythe/kythe/compare/v0.0.25...v0.0.26
[v0.0.25]: https://github.com/kythe/kythe/compare/v0.0.24...v0.0.25
[v0.0.24]: https://github.com/kythe/kythe/compare/v0.0.23...v0.0.24
[v0.0.23]: https://github.com/kythe/kythe/compare/v0.0.22...v0.0.23
[v0.0.22]: https://github.com/kythe/kythe/compare/v0.0.21...v0.0.22
[v0.0.21]: https://github.com/kythe/kythe/compare/v0.0.20...v0.0.21
[v0.0.20]: https://github.com/kythe/kythe/compare/v0.0.19...v0.0.20
[v0.0.19]: https://github.com/kythe/kythe/compare/v0.0.18...v0.0.19
[v0.0.18]: https://github.com/kythe/kythe/compare/v0.0.17...v0.0.18
[v0.0.17]: https://github.com/kythe/kythe/compare/v0.0.16...v0.0.17
[v0.0.16]: https://github.com/kythe/kythe/compare/v0.0.15...v0.0.16
[v0.0.15]: https://github.com/kythe/kythe/compare/v0.0.14...v0.0.15
[v0.0.14]: https://github.com/kythe/kythe/compare/v0.0.13...v0.0.14
[v0.0.13]: https://github.com/kythe/kythe/compare/v0.0.12...v0.0.13
[v0.0.12]: https://github.com/kythe/kythe/compare/v0.0.11...v0.0.12
[v0.0.11]: https://github.com/kythe/kythe/compare/v0.0.10...v0.0.11
[v0.0.10]: https://github.com/kythe/kythe/compare/v0.0.9...v0.0.10
[v0.0.9]: https://github.com/kythe/kythe/compare/v0.0.8...v0.0.9
[v0.0.8]: https://github.com/kythe/kythe/compare/v0.0.7...v0.0.8
[v0.0.7]: https://github.com/kythe/kythe/compare/v0.0.6...v0.0.7
[v0.0.6]: https://github.com/kythe/kythe/compare/v0.0.5...v0.0.6
[v0.0.5]: https://github.com/kythe/kythe/compare/v0.0.4...v0.0.5
[v0.0.4]: https://github.com/kythe/kythe/compare/v0.0.3...v0.0.4
[v0.0.3]: https://github.com/kythe/kythe/compare/v0.0.2...v0.0.3
[v0.0.2]: https://github.com/kythe/kythe/compare/v0.0.1...v0.0.2
[v0.0.1]: https://github.com/kythe/kythe/compare/d3b7a50...v0.0.1
