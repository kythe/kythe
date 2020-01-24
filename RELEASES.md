# Release Notes

## [v0.0.40] - 2020-01-24

#### Bug Fixes

* **java_extractor:**
  *  add the processor classpath to the extractor action (#4301) ([b079ce3e](https://github.com/kythe/kythe/commit/b079ce3eaa775b6fdf09243e2d9e4df933e66e45))
* **java_indexer:**
  *  only attempt to load implicit metadata if it exists (#4307) ([188b1cec](https://github.com/kythe/kythe/commit/188b1cec439ed4e6559a1719f5851a7a82099aca))
  *  correctly reference JVM field nodes (#4304) ([3e4c8add](https://github.com/kythe/kythe/commit/3e4c8add9aacdbde3b0c246b53fcaa33299b9227))
  *  use in-memory class_output path for modular builds (#4299) ([c8d23078](https://github.com/kythe/kythe/commit/c8d2307838c8822aac5d2786cfeb630709d0671a))
  *  handle --system none properly (#4297) ([dc585623](https://github.com/kythe/kythe/commit/dc585623a8671f11e6917da45d5c139f934fb367))
  *  handle null FileObjects in readAhead (#4290) ([0b7b623d](https://github.com/kythe/kythe/commit/0b7b623d442bfe6e436cff9a4d921c98e7390041))
  *  enable readahead in new filemanager (#4284) ([078aba28](https://github.com/kythe/kythe/commit/078aba28a5d575d059e03424ff831530ee46367c))

#### Features

* **java_indexer:**  support implicit protobuf metadata (#4262) ([cace853f](https://github.com/kythe/kythe/commit/cace853fa2d88e7a72b8608628436e4da67ff5b8))
* **kzip:**
  *  add source/input directories recursively (#4302) ([d9442588](https://github.com/kythe/kythe/commit/d9442588c2094d3be450b5a4fb49731f024d6d08))
  *  relativize input paths (#4303) ([fff4f9c3](https://github.com/kythe/kythe/commit/fff4f9c37696f78cde72c92aa8072fc5e3a4a969))

## [v0.0.39] - 2019-12-19

#### Bug Fixes

* **java:**  disable errorprone plugin for java extraction (#4268) ([08b674ea](https://github.com/kythe/kythe/commit/08b674ea71a5481bbcc9901ae214304e9fd7f658))
* **serving:**  avoid nil when serving data is missing SourceNode (#4269) ([02a41e88](https://github.com/kythe/kythe/commit/02a41e88122ae88a2e4041b766ed8c63d344b76f), closes [#4128](https://github.com/kythe/kythe/issues/4128))
* **typescript_indexer:**  emit refs to TYPE nodes in export statements (#4247) ([cd3f14df](https://github.com/kythe/kythe/commit/cd3f14df68978b6271f3bedd6f5a720351dfcc72))

## [v0.0.38] - 2019-12-12

#### Bug Fixes

*   Retain exception cause in JavaCompilationUnitExtractor (#4258) ([70120e55](https://github.com/kythe/kythe/commit/70120e5506a06f02d9c0653e00c852cc9cd5dc5b))
* **bazel extractor:**  change vnames for external paths (#4241) ([22928892](https://github.com/kythe/kythe/commit/2292889291ab88ebb796a6c4f788f691ad11218d))
* **cxx_tools:**  replace sandboxed directory with bazel execroot (#4240) ([4814f9f3](https://github.com/kythe/kythe/commit/4814f9f3fcc05c49fbe11f62f1e58a428048da27))
* **java_common:** fixes and utilities for exploded system directories (#4242) ([13f0fa7d](https://github.com/kythe/kythe/commit/13f0fa7da0959c20da243d6db4aea5dedea53f39), closes [#4213](https://github.com/kythe/kythe/issues/4213))
* **textproto:**  support direct instantiation of protobuf.Any messages (#4259) ([b64188e4](https://github.com/kythe/kythe/commit/b64188e4fb2200268397a66c99d60e95c7fe7fc5))

#### Features

* **textproto:**  index contents of google.protobuf.Any fields (#4254) ([d429a737](https://github.com/kythe/kythe/commit/d429a73788fd49b08ab15d380cdedf4892684084))

## [v0.0.37] - 2019-12-03

#### Bug Fixes

* **extractors:**  use distinct proto meta files for cc_proto_library (#4234) ([24d64419](https://github.com/kythe/kythe/commit/24d644198236d4d6b5b9fe0d2fbc74224991a0b8))

## [v0.0.36] - 2019-12-02

#### Bug Fixes

* **bazel extractor:**  give external paths a root (#4233) ([bddf41f9](https://github.com/kythe/kythe/commit/bddf41f9670d20a9554bf8b30184010763485965))
* **kzip:**  --encoding flag was previously ignored (#4216) ([0f1bea83](https://github.com/kythe/kythe/commit/0f1bea8362b692e137268034312cd6701cb9caaf))

#### Features

* **java indexer:**  better support system modules (#4218) ([a8cda826](https://github.com/kythe/kythe/commit/a8cda826b5fdf28a3ce3a9b752ed45d5c4cceb11))

## [v0.0.35] - 2019-11-15

#### Bug Fixes

* **release:**  include jsr250-api.jar in release archive (#3778) ([1d8fcb97](https://github.com/kythe/kythe/commit/1d8fcb97d20481ea614eae08d9a43a2a547f645e))

#### Features

* **java_indexer:**  plumb CorpusPaths for JVM nodes (#4205) ([d83ee7ca](https://github.com/kythe/kythe/commit/d83ee7caddf3941360e53d329710968e070698b0))
* **jvm_indexer:**  allow JVM nodes to be refined by corpus/root/path (#4204) ([952d1568](https://github.com/kythe/kythe/commit/952d15680549942322719d19c7fe6eee344f8d74))
* **serving:**  return known generators in DecorationsReply (#4201) ([486c10a3](https://github.com/kythe/kythe/commit/486c10a365c8c7afdf4a1af553bc3c123f448ae6))

## [v0.0.34] - 2019-11-08

#### Bug Fixes

* **java extractor:**  Handle source files in jars (#4188) ([df711e31](https://github.com/kythe/kythe/commit/df711e3158718a923e11ecc86e51ab4e261ddfb5), closes [#4186](https://github.com/kythe/kythe/issues/4186))
* **jvm extractor:** use class files as srcs, not jar (#4191) ([907857c7](https://github.com/kythe/kythe/commit/907857c775341728505ab7dc67bcad8112901c02))
* **kzip info:** don't count compilation units by corpus (#4195) ([88f5db4b](https://github.com/kythe/kythe/commit/88f5db4b54ba9482596dcabcb71f860f7a09fd8c))

#### Features

* **bazel extractor:**  put everything in the same corpus by default (#4192) ([3966e3f7](https://github.com/kythe/kythe/commit/3966e3f7aa790a61645e6babf511f7ae21b78011))
* **go_indexer:**  option to emit an edge for an anchor's semantic scope (#4187) ([738f5fab](https://github.com/kythe/kythe/commit/738f5fabb15cd3fa6b31cfb7347e6456d13f05aa))
* **java extractor:**  Include system modules in the compilation unit (#4194) ([b4b1b975](https://github.com/kythe/kythe/commit/b4b1b9759cff964cacc7bd0c825c264c7b1bc42b))
* **java_indexer:**  add `generates` edge to generated AutoValue sources (#4193) ([f7614b19](https://github.com/kythe/kythe/commit/f7614b19e4384c97e076821d07b4fd7f3eacf389))
* **runextractor:**  pipe underlying cmake stderr to stderr (#4178) ([aacea997](https://github.com/kythe/kythe/commit/aacea9970071ab4d7952daeab54bdf48c1d5ca2b))
* **typescript_indexer:**  add references to imports of 3rd party modules. (#4165) ([1ed21f01](https://github.com/kythe/kythe/commit/1ed21f01549f5ecc2db1181fffea85c37a86c1d1))

## [v0.0.33] - 2019-10-30

#### Features

* **release:**  Add runextractor to the release (#4171) ([5d16675f](https://github.com/kythe/kythe/commit/5d16675f41dd8874c733d760794460d472860253))

## [v0.0.32] - 2019-10-28

#### Bug Fixes

* **bazel-extractor:**  fix permissions when extracting release archive (#4168) ([063dea79](https://github.com/kythe/kythe/commit/063dea79312bab56d32d8245cd7fcdeab6ddaeec))
* **docs:**  update note on Go stdlib corpus name (#4148) ([6f79e64b](https://github.com/kythe/kythe/commit/6f79e64bf06bf40cb7fe5920c1215816799f3d7f))
* **java:**  record EntrySet emission before calling emitter (#4145) ([019f297f](https://github.com/kythe/kythe/commit/019f297f7ebd65ba572b952976b6ecb868e6e625))
* **tools:**  fix generate_compilation_database on macOS (#4166) (#4167) ([1999be3e](https://github.com/kythe/kythe/commit/1999be3ea60ca0d7c3f5db15a79519ae180f944f))

#### Features

* **kzip info:**  default --read_concurrency to numcpu (#4157) ([7fb5423d](https://github.com/kythe/kythe/commit/7fb5423d214b02eccab4174d9714dc0bc85c731e))
* **release:**  add extractors.bazelrc to release (#4163) ([b1e68e29](https://github.com/kythe/kythe/commit/b1e68e292a1da3b49cc7daebd8abd7e1a9dd5f09))

## [v0.0.31] - 2019-10-10

#### Features

* **api:**
  *  add CrossReferencesRequest.totals_method flag (#4024) ([3940fde5](https://github.com/kythe/kythe/commit/3940fde5a49600c2a40f09dae2f2d317f773da20))
  *  add build config to each related Anchor (#3738) ([b8f01f6d](https://github.com/kythe/kythe/commit/b8f01f6d4ae9e4ea3d530014f50359142791a692))
  *  allow a semantic scope to be associated per decoration (#3707) ([643cc854](https://github.com/kythe/kythe/commit/643cc854586ce4db4940ab7ab8fb068c7e44fcff))
  *  add set of build configs per Corpus in CorpusRootsReply (#3572) ([96cb400a](https://github.com/kythe/kythe/commit/96cb400a36dfa146d91fb98fad75ba66a24d1aaa))
  *  add build configurations per FileTree entry (#3523) ([b5eddd52](https://github.com/kythe/kythe/commit/b5eddd52caa6251e6543b6b6be1f0ad43784e27f))
  *  add CrossReferencesRequest filter for build config (#3512) ([cc9d2d66](https://github.com/kythe/kythe/commit/cc9d2d6650ef83832d8bbdaf86f2c6c33b8efbb6))
* **cxx_common:**
  *  support proto encoding of compilation units in kzips (#3940) ([512fe0df](https://github.com/kythe/kythe/commit/512fe0df53b6b17f5feaafa11af41494fa89111b))
  *  document and implement kythe metadata as proto (#3912) ([b19a1745](https://github.com/kythe/kythe/commit/b19a1745edfa2333a27b0d6f390fe983fb3b7244))
  *  Add ref imports edge field (#3465) ([c9c6f6e5](https://github.com/kythe/kythe/commit/c9c6f6e5c1bcd427518f0fb4282c31da08f05ee6))
* **cxx_extractor:**
  *  use a toolchain for configuring cxx_extractor (#4030) ([f1bed7e9](https://github.com/kythe/kythe/commit/f1bed7e9f9cfdc70c58838352dcaaae132035b56))
  *  allow resolving symlinks when creating vname paths (#4009) ([a5c4bc39](https://github.com/kythe/kythe/commit/a5c4bc39599adbb6b057ae48c7be1c9ea18941f8))
* **cxx_indexer:**
  *  provide defines spans for functions (#3708) ([8cd2434b](https://github.com/kythe/kythe/commit/8cd2434b921b32bc2f941431c563cfabc6b384f9))
  *  add tests and fix support for C++/proto metadata references (#3531) ([dd8fe901](https://github.com/kythe/kythe/commit/dd8fe901eef8703782c0ac6e4ad1159d1edf4a15))
* **docker_extract:**  support selecting encoding for kzip generation (#3947) ([154cec53](https://github.com/kythe/kythe/commit/154cec53334408a851ba3fb17a5e5672ecdadeb6))
  *  support package installation at extraction time to support system dependencies (#3641) ([e7d15468](https://github.com/kythe/kythe/commit/e7d15468b396e7a3926b2c601984a0ca678ecbda))
  *  Use bazelisk in Bazel extractor Dockerfile extraction. (#3596) ([94e70cde](https://github.com/kythe/kythe/commit/94e70cde56a6f2c2413a47a4dc125a5f35dd94fb))
  *  add generic GCB Bazel extraction config (#3509) ([abeff3e6](https://github.com/kythe/kythe/commit/abeff3e6a444e5d02d408f22180300cfa6a79931))
  *  add Bazel extraction Docker image (#3499) ([2af62973](https://github.com/kythe/kythe/commit/2af6297310eae2a8c95bcfbb69f722d0e338cd26))
* **extraction:**
  *  allow details to be added to a kindex required input (#4017) ([255fb53d](https://github.com/kythe/kythe/commit/255fb53d56982676cd858f94a32cc64f9561a318))
  *  allow extractor to infer corpus from sources (#45) ([bc7fc996](https://github.com/kythe/kythe/commit/bc7fc996b0f8a978a34ae66723d8c23c631c6746))
* **go extractor:**
  *  add vnames.json support (#4069) ([99cecdb6](https://github.com/kythe/kythe/commit/99cecdb661da783bd33aa9856e533775c95268c3))
  *  allow specifying go +build tags (#3851) ([b8b89d1f](https://github.com/kythe/kythe/commit/b8b89d1f6a80493d5cfad4a1fd80ee48254f88f4))
* **go_indexer:**
  *  emit parameter comments (#4134) ([93518511](https://github.com/kythe/kythe/commit/93518511aac6b3ad43143cf4a977e33621642341))
  *  emit line comments when no preferred doc is present (#4132) ([b3a278d5](https://github.com/kythe/kythe/commit/b3a278d5be022c2107f8e1b1135ab0af88cf6223))
  *  add deprecation facts for go (#3692) ([09f04420](https://github.com/kythe/kythe/commit/09f04420795ff2ce13e6b156b4ba32258e2b7d60))
  *  use LOOKUP_BY_TYPED MarkedSource child for variables (#3689) ([fc3ebd0f](https://github.com/kythe/kythe/commit/fc3ebd0f32428317837861d2cea5f1297d242f63))
  *  overriding method type satisfies interface method type (#3635) ([426b1bf1](https://github.com/kythe/kythe/commit/426b1bf1b623b49e465e978a54235aedac87d7fd))
  *  add MarkedSource for tapp/tbuiltin nodes (#3631) ([bbe98047](https://github.com/kythe/kythe/commit/bbe98047797d21e3144afa934df854da24c1e299))
  *  add receiver type to function types (#3627) ([3313fb3b](https://github.com/kythe/kythe/commit/3313fb3b45e445b1136b15b45af76f2db873c441))
  *  convert variadic function parameters from slice (#3615) ([0f638176](https://github.com/kythe/kythe/commit/0f6381768ebfa838a67d92c63e8b1bcaa1da3622))
  *  add typed edges for bindings (#3611) ([7f70f99d](https://github.com/kythe/kythe/commit/7f70f99d39aec353d3ed56ac28b72ea5a11022db))
  *  emit bindings for anonymous interface members (#3610) ([d7af13c2](https://github.com/kythe/kythe/commit/d7af13c248daa4cbe5ad333eccc4ca86a4bd8552))
* **java_common:**
  *  add forwarding method for setPathFactory (#4101) ([699004c1](https://github.com/kythe/kythe/commit/699004c16493c2f7ebdd45b5de5fcc4a82ee6a3a))
  *  use the standard compilation unit digest for kzip (#4060) ([e5ef23b2](https://github.com/kythe/kythe/commit/e5ef23b2e6180afb3f293119a59dc6773deac56a))
  *  implement standard compilation unit digesting (#4048) ([55542254](https://github.com/kythe/kythe/commit/55542254e9ef37b459c19f9f0933dfb1116277d7))
  *  rudimentary CompilationUnit FileSystem (#3928) ([8cb8d6d2](https://github.com/kythe/kythe/commit/8cb8d6d2bf38e5c6cd6d5003760cb16a9a25aa2e))
  *  add utility to convert schema proto enums (#3696) ([c0b74290](https://github.com/kythe/kythe/commit/c0b7429016d74a273847435a0855e817d253eb12))
* **java_indexer:**
  *  Record @deprecated javadoc tags in the Java indexer (#2977) ([84a77bf9](https://github.com/kythe/kythe/commit/84a77bf9671833b3044c390f5b565fa7464aefff))
  *  move path-based file manager config to base class (#4133) ([831b73ec](https://github.com/kythe/kythe/commit/831b73ec8f38951a65c94ededbe198cb5279df5c))
  *  command line option to use path-based file manager (#4122) ([01858aba](https://github.com/kythe/kythe/commit/01858aba8883aa6d6cf49e286192347daf5a21a8))
  *  Initial path-based JavaFileManager implementation (#4115) ([800002ca](https://github.com/kythe/kythe/commit/800002ca88e246505ea6f56caa60f76e87aa252c))
  *  add typed edges for `this`/`super` (#4058) ([dceb969d](https://github.com/kythe/kythe/commit/dceb969d435453a0be71d44fdef7f46bfaee6d50), closes [#4055](https://github.com/kythe/kythe/issues/4055))
  *  add property/* edges for @AutoValue classes (#3993) ([0abf28c9](https://github.com/kythe/kythe/commit/0abf28c9123a853edff350c1b4d4c254619006c0))
  *  add basic tests for java-9-10-11 features (#3898) ([2139771c](https://github.com/kythe/kythe/commit/2139771cc23b12f93f8d439d349ca6dbc9d1e5ec))
  *  emit refs to JVM graph for externally defined nodes (#3800) ([0d7614ed](https://github.com/kythe/kythe/commit/0d7614eda17a0d659b120d6479965d73ad861093))
  *  add receiver type to function types (#3628) ([fe7aa759](https://github.com/kythe/kythe/commit/fe7aa7596bedb248ec6f8f0cd440a23d8eafbdc9))
  *  add MarkedSources for function tapp nodes (#3601) ([f36eceff](https://github.com/kythe/kythe/commit/f36eceff4dc4376aed1ed5bcaa4bfc038a81aa6a))
  *  add MarkedSources for array/generic tapp nodes (#3561) ([e2aaf23c](https://github.com/kythe/kythe/commit/e2aaf23c788f781bfdd2b3a2a30274d920192c73))
* **jdk_indexing:**  allow extracting only specific make targets (#3935) ([2978be61](https://github.com/kythe/kythe/commit/2978be6173622481b2761a9ea4869f54b2a1fbdf))
* **post_processing:**
  *  support Riegeli Snappy compression (#4072) ([a7631889](https://github.com/kythe/kythe/commit/a76318890a9373be509f3478f946c703dc60e175))
  *  add Nodes.fromSource conversion utility (#3755) ([03bdb570](https://github.com/kythe/kythe/commit/03bdb57052655c8c352ccb6dfdd82ad33946b474))
  *  allow nodes/anchors to be ranked (#3750) ([466b7bc4](https://github.com/kythe/kythe/commit/466b7bc4e4642b953c6285264cdf31f11f0f8686))
  *  implement basic callers in PagedCrossReferences (#3712) ([87f273d1](https://github.com/kythe/kythe/commit/87f273d184cae9d47684cb6a58e95ef842e82ea8))
  *  support reading input files from a directory (#3497) ([e907832f](https://github.com/kythe/kythe/commit/e907832f605731117ff5cd0b92c3e582806512e5))
* **proto:**
  *  Add standalone proto extractor (#21) ([c1c53ce4](https://github.com/kythe/kythe/commit/c1c53ce41624166e0eb1540cda243d38ac430d3b))
  *  add extra_action and action_listener (#43) ([956304df](https://github.com/kythe/kythe/commit/956304df4bfb584c5b6655f44cb395c6c00928b1))
  *  emit deprecation tags for proto fields (#3677) ([418a3d9a](https://github.com/kythe/kythe/commit/418a3d9a451cd779a0e0b0576ba4069008956658))
  *  proto plugin to embed metadata in C++ headers (#3511) ([0d2ea56e](https://github.com/kythe/kythe/commit/0d2ea56e8754056f86bfd177ee4e419316c8df7c))
* **schema:**
  *  embed metadata in schema.proto source file (#4020) ([902b1344](https://github.com/kythe/kythe/commit/902b13444bd6eacb43a4f7be3487eabad32e97ce))
  *  add Java utility to convert Node/Entry protos (#3714) ([19567cea](https://github.com/kythe/kythe/commit/19567ceadbffbe3d1ba9c481568a84e5daebeb7f))
  *  add schema proto representation of a Kythe Entry (#3704) ([058565aa](https://github.com/kythe/kythe/commit/058565aa7f6dd412811e6b35792a866f044b854f))
* **serving:**
  *  add wildcard node kind support to xrefs indirection (#3984) ([2f8a5bcf](https://github.com/kythe/kythe/commit/2f8a5bcf17eb4a43a8f78e156328fc746820ea0e))
  *  experimental support for xrefs indirection (#3980) ([bfb2c24c](https://github.com/kythe/kythe/commit/bfb2c24c3e524d15aa2700c063fb2c0f6a865fcc))
  *  add build configs per corpus (#3580) ([0292676f](https://github.com/kythe/kythe/commit/0292676ff9b93246a9e87e070dd70cd56def2edb))
  *  add build configs to FileTree serving data (#3527) ([448d46e8](https://github.com/kythe/kythe/commit/448d46e8bb30c16345d516f09b6904e92b7a05cf))
* **textproto_indexer:**
  *  add indexer for text-format protobuf (#39) ([d9c96147](https://github.com/kythe/kythe/commit/d9c96147cfb2affeaec4fb39e905033d3ae8f10d))
  *  add standalone extractor for textproto (#35) ([72cd0129](https://github.com/kythe/kythe/commit/72cd0129a580ec7fc494b8a65e2b1e0c5168a052))
  *  add library for parsing textproto schema comments (#31) ([b6f38f05](https://github.com/kythe/kythe/commit/b6f38f05feaeeea1e38ceeeb92bee849926921cd))
  *  support inline repeated field syntax (#46) ([832e1583](https://github.com/kythe/kythe/commit/832e15836697caff7cc497f5de546b7b9cc851d2))
  *  add refs for schema comments (#53) ([93e69912](https://github.com/kythe/kythe/commit/93e699124688438ad4985369a220938e0fb496d9))
* **tooling:**
  *  implement (*riegeli.Writer).Position (#3919) ([94637264](https://github.com/kythe/kythe/commit/94637264a2118c71455f6a3a6ee93b114249284e))
  *  add --semantic_scopes flag to `kythe decor` command (#3719) ([bd17d082](https://github.com/kythe/kythe/commit/bd17d08293e84f459d41cafa3902a94f45963d7d))
  *  add --build_config filter to 'kythe xrefs' command (#3588) ([8cd01247](https://github.com/kythe/kythe/commit/8cd01247759158c63db0fdaaba321e45183278e7))
  *  add --build_config filter to 'kythe decor' command (#3587) ([9cae15ee](https://github.com/kythe/kythe/commit/9cae15ee52f7bddb5a439ee521f2bacf93ecfa3f))
  *  add KytheFS tool to mount index to filesystem (#3419) ([cddcda7c](https://github.com/kythe/kythe/commit/cddcda7c0553f5f8c8105f7712e58a01fbabaf19))
  *  migrate kythe CLI to new filetree API (#3487) ([59aee1e3](https://github.com/kythe/kythe/commit/59aee1e3a0a109470944299c4ebefdcf986145e6))
  *  kzip can merge into the existing output file (#3679) ([ea0f3de7](https://github.com/kythe/kythe/commit/ea0f3de7dba206b10d145c5149a1eddacfdbffaa))
  *  add kzip subcommand for creating trivial kzips (#3537) ([dc839a1e](https://github.com/kythe/kythe/commit/dc839a1ecbff887fe7bf016e5d25b7855c718786))
  *  add function for merging multiple KzipInfos together (#4084) ([e2202c5a](https://github.com/kythe/kythe/commit/e2202c5ad95830fd0c6e4b92e5a93698cac8703c))
  *  change Unit.Digest() to return hex string representation (#4044) ([f81e09b3](https://github.com/kythe/kythe/commit/f81e09b316c2cdef72a16bc63140504cd61ea8fa))
  *  thread encoding into kzip_extractor bazel rule (#4027) ([3f81db7f](https://github.com/kythe/kythe/commit/3f81db7f348879d969d5142861eea6edf688c371))
  *  add filter subcommand to kzip tool (#3998) ([7d4e7de4](https://github.com/kythe/kythe/commit/7d4e7de4dcdcf24cae76ecd6f6e4263dfb37d125))
  *  add `kzip view` command (#3957) ([5d4b4003](https://github.com/kythe/kythe/commit/5d4b40033161741720c78d33c9fe42e451a69b93))
  *  support both proto and JSON encodings in Java for compilations in kzips. Also move kzip testdata to common location. (#3852) ([4d86c48d](https://github.com/kythe/kythe/commit/4d86c48de9a10f108b8eb42eb8e7aeca68044ad7))
  *  create permits specifying encoding (#3842) ([18ad24ed](https://github.com/kythe/kythe/commit/18ad24ed16afb18a4d01e9a3396e3f710e06146e))
  *  add support for encoding compilation units as proto and/or JSON in kzips. (#3836) ([3643205e](https://github.com/kythe/kythe/commit/3643205ee560df95be9944a719ce937e3ad60de2))
  *  add `info` command to kzip tool which returns count of compilation units and referenced corpus names. (#3840) ([6f83e91f](https://github.com/kythe/kythe/commit/6f83e91f3a412193756704535a73c66097badf22))
  *  add option to read compilation units in parallel (#4096) ([1db42c19](https://github.com/kythe/kythe/commit/1db42c192231774f5a30adcbaa18827ad417162c))
  *  add proto output format to `kzip info` (#3991) ([d32202eb](https://github.com/kythe/kythe/commit/d32202ebd959cfabc9da546b2cfe980a842c2a8c))
  *  use vfs to allow for changing filesystem impl (#3971) ([cb882e9a](https://github.com/kythe/kythe/commit/cb882e9a78065cba6e00e3893490e798c59367a3))
  *  add more details and json output mode to `kzip info` (#3933) ([09cb07d9](https://github.com/kythe/kythe/commit/09cb07d922ffffd0e1ac5a3eb36212b833a2ebc1))
  *  allow complete specification of a compilation unit (#3542) ([7f82fe42](https://github.com/kythe/kythe/commit/7f82fe42d27d10819f56bbfea0d791cff3013642))
  *  allow embedding details messages in kzips (#3538) ([d0255f56](https://github.com/kythe/kythe/commit/d0255f56a0d6336469aff463ff032d40f0d30176))
* **typescript:**  Support AngularTemplateCompile mnemonic (#4105) ([98808c0a](https://github.com/kythe/kythe/commit/98808c0ac93ffb1dbd8ab899b5c30e90c74c8da1))
* **typescript_indexer:**
  *  don't emit duplicate nodes for namespaces (#4062) ([3abf05cf](https://github.com/kythe/kythe/commit/3abf05cff5ba8eaafc0107238745645d6aaad851))
  *  print plugin names (#4061) ([f4f15815](https://github.com/kythe/kythe/commit/f4f158153c5e9eff47b9e6833e17ec716c25deb8))
  *  emit references to hops in imports (#3990) ([e385c8c3](https://github.com/kythe/kythe/commit/e385c8c3bba45d42e147fb93a1fcb0a8f8c8a669))
  *  add minimal support for JSX (#3888) ([c9f999d0](https://github.com/kythe/kythe/commit/c9f999d0fe37d5a064dcdff93d193befd84903b7))
  *  expose getSymbolName through IndexerHost (#3909) ([6d55e1e0](https://github.com/kythe/kythe/commit/6d55e1e02f54157ad48bddbbc7c8da34634dbe56))
  *  add support for utf-16 offset lookup (#3908) ([eae1ff81](https://github.com/kythe/kythe/commit/eae1ff810d01eea710feae48e3b1518a72c06a02))
  *  add support for indexing literal properties (#3899) ([b4d64007](https://github.com/kythe/kythe/commit/b4d640072731e54162b89eca2d4db007b8be3cf6))
  *  define VName specification (#3783) ([ca76e298](https://github.com/kythe/kythe/commit/ca76e29846b22021e929d0285d8f06e024ec45c9))
  *  emit "aliases" edge on aliased types (#3870) ([7dd02837](https://github.com/kythe/kythe/commit/7dd02837849bfa0378b015b475407844101957b3))
  *  emit "overrides" edges for overridden methods (#3872) ([ce043510](https://github.com/kythe/kythe/commit/ce043510f1c1f79b900dfef1ecfca4f8c359c696))
  *  index export equals and import equals nodes (#3871) ([1c54c8a1](https://github.com/kythe/kythe/commit/1c54c8a1ad82bdb38da2cc28b53071835606adbc))
  *  add support for indexing module decls (#3873) ([dd53f6b1](https://github.com/kythe/kythe/commit/dd53f6b11187ab5515d19c7acbc0fa92e4fd42ea))
  *  add xrefs for declarations in objects (#3864) ([e7e41b67](https://github.com/kythe/kythe/commit/e7e41b67484d86d18cc1888e5cb5acdb69ed54a3))
  *  reference modules in export statements (#3863) ([f3eb65f1](https://github.com/kythe/kythe/commit/f3eb65f1a7e6e09f1cf5adf8d8bc00541c6cf264))
  *  pass module name creation logic to plugins (#3828) ([5756795f](https://github.com/kythe/kythe/commit/5756795fafe53c388a7f4774dce34fc2b542674e))
  *  VName schema compliance for constructors (#3785) ([cdee8bf8](https://github.com/kythe/kythe/commit/cdee8bf8a813ddd0309dbab00110f0e972282996))
  *  Getter/Setter entries (#3784) ([0e2dd884](https://github.com/kythe/kythe/commit/0e2dd884987ec3bbe95db19e6f5a3e667128aa61))
  *  add `defines` anchors for functions (#3775) ([f03ecc89](https://github.com/kythe/kythe/commit/f03ecc89817bdbfd7d2315aab2d6271eafaa0a17))

#### Bug Fixes

*   bump nokogiri to 1.10.4 (#4007) ([870a5830](https://github.com/kythe/kythe/commit/870a58307f7bd3506d32405b7ef37b54b3a4d4d7))
*   update vulnerable outdated js-yaml NPM package (#3740) ([b873a1c6](https://github.com/kythe/kythe/commit/b873a1c676ce204ff8e66ffe765531c1f544b68e))
*   get toolchains for remote builds working correctly (#7) ([b034a8f3](https://github.com/kythe/kythe/commit/b034a8f360f7279b8a0025e91a24fb394c5c7e91))
* **api:**  filter decoration overrides by desired build configs (#3739) ([3f4ef51b](https://github.com/kythe/kythe/commit/3f4ef51b7ba9c347ebc7bca151d19a6bc55a5309))
* **cxx_common:**
  *  fix path_utils_test when tempdir is a symlink (#4022) ([f71af3dd](https://github.com/kythe/kythe/commit/f71af3dd8ca3975698c304565d9ccdc00154de85))
  *  update LLVM to r367622 (#3944) ([1d7123f9](https://github.com/kythe/kythe/commit/1d7123f9e3d5c79d72ebac600e9d8e5bfa654b7b))
  *  anticipate changes in Bison 3.3 syntax (#3513) ([d14cb507](https://github.com/kythe/kythe/commit/d14cb50782d1056472615f2265e939df778243f5))
* **cxx_extractor:**
  *  use the right compiler executable (#4032) ([a2015717](https://github.com/kythe/kythe/commit/a2015717012b1f334b1c8e8880ac140a34773744))
  *  better support cuda extractions (#3747) ([2d7f89c6](https://github.com/kythe/kythe/commit/2d7f89c69a1cc816d21c7d86faf6791d4afda8e7))
  *  include executable name in extracted arguments (#4023) ([58a62498](https://github.com/kythe/kythe/commit/58a62498b30691d63a621ae119c07d750c71cdc5))
  *  Set cxx extractor kzip times to a constant (#4086) ([642c9330](https://github.com/kythe/kythe/commit/642c933026acafd44f49de81f12e9f21e4fb6de0))
* **cxx_indexer:**
  *  emit references to dependent names based on name, not location (#4097) ([28cf259b](https://github.com/kythe/kythe/commit/28cf259beac09d1a4d687f87990ddba92ec32bff))
  *  whitelist and test std::make_shared (#4059) ([4016c5b3](https://github.com/kythe/kythe/commit/4016c5b30a02a85e996c10e4a873e10d1b80c4cd))
  *  allow protobuf metadata in .stubby.h files (#4004) ([29cdef09](https://github.com/kythe/kythe/commit/29cdef09e32b5fc86420e1774ff2d79067dc08a8))
  *  properly reference C++17 deduced templates (#3900) ([69ef278c](https://github.com/kythe/kythe/commit/69ef278c8cc262df5b3437d468e8e304a44b2ad3))
  *  normalize windows paths (#3742) ([167786ac](https://github.com/kythe/kythe/commit/167786acf6f2d10e1e8f3923da0a1f4f59e59a66))
  *  insist that compilation units have working directories (#3716) ([d3e059d1](https://github.com/kythe/kythe/commit/d3e059d1ecbb74ffe61f8d47ede71e2b244d5933))
  *  always add . to the VFS (#3715) ([a8e95930](https://github.com/kythe/kythe/commit/a8e9593033ed63573948d047e3afc4e31f400d99))
  *  use URI-friendly encoding for VName fields (#3705) ([f9a7e299](https://github.com/kythe/kythe/commit/f9a7e299c8f89316e8a09635b143023a1950d6d3))
  *  use build_config when claiming implicit AST subtrees (#3619) ([8868db03](https://github.com/kythe/kythe/commit/8868db03a0d456ce93f2b22b09a4a82267d685b7))
  *  attach build_config to file claim tokens (#3590) ([05800883](https://github.com/kythe/kythe/commit/058008830d4bc4cbb693c0146685d10fbd2a93a4))
* **cxx_verifier:**  fix MSAN error on use-of-uninitialized (#4010) ([08fa1823](https://github.com/kythe/kythe/commit/08fa18231b1d52da3788471ad6f45e52720ccd54))
* **go extractor:**
  *  fix invalid Details messages for packages (#4120) ([426a9b0b](https://github.com/kythe/kythe/commit/426a9b0be61ff6eb35a9bc94fdd4e73d79e28138))
  *  Change action go bazel extractor runs on (#4087) ([27626173](https://github.com/kythe/kythe/commit/276261739e7cbe4336e04c4cb01354086d9d2754))
  *  -tags is space-separated, not comma (#3856) ([c932970f](https://github.com/kythe/kythe/commit/c932970f0e11b0f8198a137535bc80a7ffe8341a))
* **go_common:**  use nondeprecated zip file handle mod time (#4047) ([b9e9c011](https://github.com/kythe/kythe/commit/b9e9c01155d91df10833c2faafe769f35f9b6aba))
* **go_indexer:**
  *  use only name for a named type's MarkedSource (#3850) ([f363e506](https://github.com/kythe/kythe/commit/f363e50690b65727ba33a8e09a57b177788b24db))
  *  handle overrides for structs in a other packages (#3606) ([f3364504](https://github.com/kythe/kythe/commit/f3364504337e26e5a407a63c9c6f947b22b72cb3))
  *  always emit anchor for anon value types (#3605) ([d97ca443](https://github.com/kythe/kythe/commit/d97ca443b265685c10e7caa7d4cc1c9f97933ee7))
* **java:**
  *  handle more null data from java (#4082) ([e878b5f5](https://github.com/kythe/kythe/commit/e878b5f58a8a7174b2c83373b89f60b99ce9b092))
  *  accept either JRE 8 or JRE 9 style classpath in test (#3972) ([3ea7de6c](https://github.com/kythe/kythe/commit/3ea7de6ccd55e30a693e60996468a14f78151630))
* **java_build:**  remove jvm_flags which complicate running on newer JDKs (#3955) ([b4bdb395](https://github.com/kythe/kythe/commit/b4bdb395bd108415e5d8deab65a6538915d31a6c))
* **java_common:**
  *  normalize paths before looking them up (#4106) ([d4155894](https://github.com/kythe/kythe/commit/d4155894a82c556742250a17868abfb16625ef26))
  *  adjust case when getting default format (#4104) ([64659661](https://github.com/kythe/kythe/commit/646596615567fb3d63d5d50c29bdfe9896cd4524))
  *  add file digest as Path URI host (#3975) ([2ead1aa4](https://github.com/kythe/kythe/commit/2ead1aa48f6828cdf91a97300edd0728abae884d))
  *  allow resolving relative paths against CU working dir (#3937) ([8050ceec](https://github.com/kythe/kythe/commit/8050ceeca656443266885c879f0afb635869d452))
  *  replace or remove JDK9+ methods (#3930) ([b1810d84](https://github.com/kythe/kythe/commit/b1810d84044a8d654c299b8f27e80eb0ae837be6))
* **java_extractor:**
  *  include working directory in extracted proto (#4130) ([bc363f87](https://github.com/kythe/kythe/commit/bc363f877d35263dcbd0112d93a443e66481d2c1))
  *  and tests should use ambient langtools (#4053) ([7f680e94](https://github.com/kythe/kythe/commit/7f680e94bb60ca23219421f6479dbf1c63a8f37b))
  *  support missing -s in aspect extraction (#3986) ([7609b970](https://github.com/kythe/kythe/commit/7609b9700e6a30a5ec052e9baa713e59f073bee0))
  *  deal with both kinds of Generated annotations (#3889) ([048134fd](https://github.com/kythe/kythe/commit/048134fd919bd263d996c22c226df70955bf23c7))
  *  remove Option.D from compilation unit again (#3670) ([8a0bdfda](https://github.com/kythe/kythe/commit/8a0bdfda6346fc0cb33ac0d842bba536e40347ec))
  *  use reflection to access/override JDK bits. (#3612) ([72915498](https://github.com/kythe/kythe/commit/72915498ba2b803b180543db35bcfa894a2c4c03))
  *  Don't include time in kzips for java (#4085) ([3c25f1a0](https://github.com/kythe/kythe/commit/3c25f1a07109179ace64dc3517a49b7f4b07dbc0))
* **java_indexer:**
  *  cast to base with asPath to support JDK8 (#4137) ([1ab8b883](https://github.com/kythe/kythe/commit/1ab8b8831a0e711e9334d322cbccc90fc56bdecf))
  *  address part of #3459 by claiming comments for annotation lines (#3489) ([bcddb97c](https://github.com/kythe/kythe/commit/bcddb97c57a97dcd0cc1580c5213f899a2d8230b))
  *  use Path to resolve metadata (fixes #4135) (#4136) ([e03d4726](https://github.com/kythe/kythe/commit/e03d47265ea4b60b369bc420245ec1a73a953814))
  *  accept AutoValue.Builder interface definitions (#4054) ([e12eee78](https://github.com/kythe/kythe/commit/e12eee78695c8881eccb32e9ac608020f3804f27))
  *  implement JDK9+ path-based FileObject methods (#3976) ([bb70a39f](https://github.com/kythe/kythe/commit/bb70a39fd24d353f0f1d481e1d5822fc0f461361))
  *  dynamically resolve JRE 9 bootclasspath (#3970) ([f95e4395](https://github.com/kythe/kythe/commit/f95e4395998737b8b5d769c28682b39a9b75da38))
  *  emit and test type parameter MarkedSources (#3832) ([7cb46e3f](https://github.com/kythe/kythe/commit/7cb46e3f95b7795a4b597db48bda41a1290a5815))
  *  emit MarkedSource for defined nodes referenced earlier in analysis (#3826) ([e3d031cc](https://github.com/kythe/kythe/commit/e3d031cce4f5e7e5c1ef38aa5078abc8b4726702))
  *  avoid NPE when referencing Java package in comment (#3803) ([42f94049](https://github.com/kythe/kythe/commit/42f940495360beb4f83f9e08c365941b2a4e0bc7))
  *  only emit MarkedSource for defined symbols (#3792) ([3079f619](https://github.com/kythe/kythe/commit/3079f6199e30f34cc7911af56cb9e79f0ef1bb1c))
  *  only emit MarkedSource for defined symbols (#3788) ([6bfb2f7a](https://github.com/kythe/kythe/commit/6bfb2f7a46e7704f25073b7f419c34062218765a))
  *  box LOOKUP_BY_PARAM MarkedSources with text (#3632) ([44771993](https://github.com/kythe/kythe/commit/44771993f97a7bd61b64a75884595ea3979baead))
* **jdk_extractor:**  bazel runfiles no longer expect the workspace name (#4131) ([ae19c938](https://github.com/kythe/kythe/commit/ae19c938a79218865f9dd320865397f5a15d05d9))
* **pipeline:**  beam output path can contain a filesystem scheme (#3532) ([eba6ff48](https://github.com/kythe/kythe/commit/eba6ff4817a3456ee2bc1baac31fbb569919598b))
* **post_processing:**
  *  split semantic callers within xrefs group (#3776) ([87e1e2ed](https://github.com/kythe/kythe/commit/87e1e2ed393dad8abb0865ff7fdc36eab81e6f60), closes [#3768](https://github.com/kythe/kythe/issues/3768))
  *  convert DirectoryReply to FileDirectory for serving data (#3604) ([0ef9b748](https://github.com/kythe/kythe/commit/0ef9b7487088173ba66916eb84f199642eab5949))
  *  make pipeline more tolerant to some errors (#3490) ([dc33424b](https://github.com/kythe/kythe/commit/dc33424b08fda5c6dbdfbc4598851e32cf95833e))
* **proto:** 
  *  include deprecation tags for messages, enums and enum values. (#3822) ([c602555d](https://github.com/kythe/kythe/commit/c602555d75c8cff60722027eb99062b5b07c6153))
  *  link BuildDetails proto (#49) ([e9b68f17](https://github.com/kythe/kythe/commit/e9b68f1708ebeb4c11e0a7ff155ab0a5480d3a35))
  *  proto fields have subkind "field" (#4098) ([ac6cb14b](https://github.com/kythe/kythe/commit/ac6cb14b3a1d7d91abbfcc94a879b8b3ca961f4e))
  *  empty deprecation message and additional test case (#3678) ([d56bc1d3](https://github.com/kythe/kythe/commit/d56bc1d3f43badb99c313288c50aa8109b2e141d))
  *  skip synthesized map entry types (#26) ([c23d4d52](https://github.com/kythe/kythe/commit/c23d4d5271cb22174328a6b77be6086c7dd55c22))
  *  close file after use in SourceTree.Read() (#18) ([da1c9cc0](https://github.com/kythe/kythe/commit/da1c9cc0779b92748249be34d2d455934f6943a1))
  *  extract test files in TEST_TMPDIR instead of runfiles dir (#29) ([f94a1299](https://github.com/kythe/kythe/commit/f94a1299e12b25e9e363df194edcaa15f1bdfb45))
* **serving:**
  *  properly return lookup errors (#3764) ([d5c0e4ec](https://github.com/kythe/kythe/commit/d5c0e4ec7a5505f3f63a76a1ec53a860319163b0))
* **tests:**
  *  make proto/textproto tests not rely on `jq` (#3967) ([28986284](https://github.com/kythe/kythe/commit/289862842f3c08317f531d0f064e8880a22b321b))
  *  remove deadline option from proto testdata (#8) ([7f2f889d](https://github.com/kythe/kythe/commit/7f2f889d3409feb2bec7d73ffe5b3ac2647f89c0))
* **textproto_indexer:**
  *  continue analysis even if schema comments are invalid (#3859) ([07e5eda4](https://github.com/kythe/kythe/commit/07e5eda4261f488b42033eb546d5230f379e77d8))
  *  handle relative paths correctly in textproto and proto extractors (#3694) ([dbaa0a8b](https://github.com/kythe/kythe/commit/dbaa0a8b9c062384b274ff42fcfa59809fff6eda))
  *  implement and test repeated extension fields (#50) ([e658b1cb](https://github.com/kythe/kythe/commit/e658b1cb1d8a704e69a08fba5ab7f8c82217d09f))
  *  improve error handling around optionals (#42) ([24455b8a](https://github.com/kythe/kythe/commit/24455b8a9a1b8c6c892065cbace50a866e8aff6a))
* **tooling:**
  *  do not consider the empty kzip an error (#4111) ([7dc45432](https://github.com/kythe/kythe/commit/7dc45432a46c138d6363b91dcdca4e83f1cb33a0))
  *  check error before result (#4110) ([3207a48f](https://github.com/kythe/kythe/commit/3207a48f4d89d8a50e80984da437e944e4e66c23))
  *  use uncompressed riegeli block size when decompressing (#3997) ([a6040e85](https://github.com/kythe/kythe/commit/a6040e8507514fb3dc2963b854f8856fb6de01dd))
  *  correct chunk position starting a block boundary (#3782) ([341fd2cf](https://github.com/kythe/kythe/commit/341fd2cf93d484941f25e5f2e6c60276e8c2786c))
  *  fix #3246 (#3737) ([c2d7ee67](https://github.com/kythe/kythe/commit/c2d7ee679e295bedb2a274b408e04a75d18213ac))
  *  allow sample UI dependencies to be disabled (#3717) ([4c687714](https://github.com/kythe/kythe/commit/4c68771432d0e7335fcee427fdd1ef5bbe95ea8d))
  *  migrate 'kythe ls' filters from deprecated FileTree API (#3566) ([5c3e9c27](https://github.com/kythe/kythe/commit/5c3e9c27ea899a49e730c2a47e28515f11ea46b5))
  *  migrate kythefs from deprecated FileTree API (#3564) ([3ffd2597](https://github.com/kythe/kythe/commit/3ffd25975f6bc0343b52bcfe259dda5c25427a97))
  *  populate KzipInfo.total_files in info.go (#4018) ([d1b99218](https://github.com/kythe/kythe/commit/d1b9921820b8992a0157473e319eb0bccf5dfd11))
  *  make default kzip encodings explicit (#4012) ([b526e547](https://github.com/kythe/kythe/commit/b526e5474ef2185a9081f20dacfab1c8d19a182f))
  *  use proper incantation so testdata still found on import (#3846) ([06f9aa46](https://github.com/kythe/kythe/commit/06f9aa46fac3adc65f732bc8fad898782c29e9cd))
  *  EncodingFlag fully implements flag.Getter and flag.Value (#3843) ([55cf0dd2](https://github.com/kythe/kythe/commit/55cf0dd2a69840d0a66a4057c31f37957e8cf82a))
  *  determine unit corpus and file language more robustly (#4095) ([c4a354e8](https://github.com/kythe/kythe/commit/c4a354e835acef2b9eb14d026142f440b5871fd0))
  *  implement flag.Getter for all flat types (#3544) ([a47b32ed](https://github.com/kythe/kythe/commit/a47b32ed1f6a747a0606cad38ac30340386a056a))
  *  Don't include kzip creation time in kzip (#4083) ([5092d9d5](https://github.com/kythe/kythe/commit/5092d9d5a0a01131e29ad59dce791815a71b85ec))
* **typescript_indexer:**
  *  add anon signature part for function expressions (#3880) ([b805d3a9](https://github.com/kythe/kythe/commit/b805d3a96fb4a0ef80e3294e98822f2636337e4a))
  *  don't throw on unfound module path (#3882) ([e8f2c5ec](https://github.com/kythe/kythe/commit/e8f2c5ecf333da124ce9eee501c8082e20246aa6))
  *  do not give anonymous names to binding patterns (#3857) ([f4fa3d58](https://github.com/kythe/kythe/commit/f4fa3d58187688e8d3e6570e1984b3d6bc0e2976))
  *  Do not name scopes created by rvalues (#3841) ([4e6d241f](https://github.com/kythe/kythe/commit/4e6d241f8319c5a31c8d4e8d74bafe1b35cd0bc5), closes [#3834](https://github.com/kythe/kythe/issues/3834))
  *  differentiate signatures of static and instance members (#3819) ([74e1e4a2](https://github.com/kythe/kythe/commit/74e1e4a2771a7c58ddbd56a00fcf0505b6c082a1))
  *  point class type and values at the class name (#3835) ([35f41ccd](https://github.com/kythe/kythe/commit/35f41ccd0cc1d789f811de672fa68f097f8d4483))
  *  remove deprecated childof edge from anchor to file (#3724) ([5533e861](https://github.com/kythe/kythe/commit/5533e8612cb0913906c45345c71bdd290296e635))
  *  properly lookup file VNames (#3686) ([34369679](https://github.com/kythe/kythe/commit/3436967916f58850d7b837230d04207c08e1d51d))
  *  obey VNames that come in from the input (#3467) ([a2bc9861](https://github.com/kythe/kythe/commit/a2bc9861d46403a47e38ae6506bba4ce7c98e0e2))

#### Performance

* **post_processing:**  use heap.Fix in disksort (#4063) ([3cfd3b3b](https://github.com/kythe/kythe/commit/3cfd3b3bf147b6d95671e2c74e8866dfcc6d8596))

## [v0.0.30] - 2019-02-07

#### Features

* **api:**
  * add DecorationsRequest filter for build config (#3449) ([2480a935](https://github.com/kythe/kythe/commit/2480a9357b755fb204d7847bcd42626263226f5a))
  * structured entries in DirectoryReply (#3425) ([53e3a097](https://github.com/kythe/kythe/commit/53e3a0978b478eb716dd88ee70ae69f97f426bc7))
*  **build_details:**  add a build_config field to BuildDetails proto (#3303) ([ed5ce4d5](https://github.com/kythe/kythe/commit/ed5ce4d53c5714cb3f56d876842ef54c92eb1063))
* **columnar:**
  *  add Seek method to keyvalue.Iterator interface (#3211) ([753c91ae](https://github.com/kythe/kythe/commit/753c91ae536c0a803c7a10d7900a6c9af2c38fc4))
  *  support build_config filtering of columnar decorations (#3450) ([810c9211](https://github.com/kythe/kythe/commit/810c9211df728b447e7d070002e9d8d2857eed0a))
  *  add definitions for related node xrefs (#3228) ([52a635eb](https://github.com/kythe/kythe/commit/52a635eb80ba2b31d8d6eb6b37e855b3fe49a0ae))
  *  handle columnar decl/def CrossReferences (#3204) ([eec113df](https://github.com/kythe/kythe/commit/eec113df604ae70c75ecbd0f941930e45b91131e))
* **cxx_indexer:**
  *  copy in utf8 line index utils (#3276) ([93e69d29](https://github.com/kythe/kythe/commit/93e69d29d6ccd801c7067907c3f0dd4b0c6eed8b))
  *  support emitting USRs for other kinds of declarations (#3268) ([4d705cf9](https://github.com/kythe/kythe/commit/4d705cf9b55fe00d286e159d63b7ca35d0885a52))
  *  Support adding Clang USRs to the graph (#3226) ([15535c65](https://github.com/kythe/kythe/commit/15535c65f50b7a0e867d3aca9a5d71d1c302b8c9))
  *  include build/config fact on anchors when present (#3437) ([96c7d6bc](https://github.com/kythe/kythe/commit/96c7d6bcbead16fa3a99ce03750e413a7b901d3c))
  *  Add support for member pointers and uses of them a… (#3258) ([a83e856d](https://github.com/kythe/kythe/commit/a83e856d789b375bfef5ee9dad27169c121d7cd6))
  *  read all compilations from a kzip file (#3232) ([3bd99ee7](https://github.com/kythe/kythe/commit/3bd99ee7c251298cfdcb3860ea16ca54e47b66c4))
* **gotool_extractor:**
  *  canonicalize corpus names as package/repo roots (#3377) ([f2630cbc](https://github.com/kythe/kythe/commit/f2630cbcf51e3b301bbbf3c3e716f4365dd159e6))
  *  add Docker image for Go extractor (#3340) ([f1ef34b5](https://github.com/kythe/kythe/commit/f1ef34b5e227e4807ae65173ff0e1e325971fe9e))
  *  use Go tool to extract package data (#3338) ([a97f5c0e](https://github.com/kythe/kythe/commit/a97f5c0e5928d7ac4cc2dcf7856731bd111e3f50))
* **java_indexer:**  emit implicit anchors for default constructors (#3317) ([90d1abfe](https://github.com/kythe/kythe/commit/90d1abfe6b45cf6aa1050f03dbfdd5fe7d4a7d8b))
* **objc_indexer:**
  *  support marked source for @property (#3320) ([b79d49bd](https://github.com/kythe/kythe/commit/b79d49bd6805901f77f8bab58cd108ec046e6c47))
  *  marked source for category methods (#3311) ([414bdf2d](https://github.com/kythe/kythe/commit/414bdf2de1443b7f60becddd99c0619bbf9506f1))
* **post_processing:**
  *  limit disksort memory by custom Size method (#3201) ([7919fcf1](https://github.com/kythe/kythe/commit/7919fcf1d5c6e182d3db16ed48c1caa4186e7ff8))
  *  pass-through build config in pipeline (#3444) ([59be834f](https://github.com/kythe/kythe/commit/59be834f3ed1600fc1dcbe5d3fbdafc27c3aa2bd))
  *  add build configuration to anchors in serving data (#3440) ([e3c7fa18](https://github.com/kythe/kythe/commit/e3c7fa18eaee9315834a51a4c0f46678c9d98138))
  *  add diagnostics to file decorations (#3277) ([0cd5dfca](https://github.com/kythe/kythe/commit/0cd5dfcad100d40275690ff3d1f2e4147c0212ba))
  *  add support for Riegeli input files (#3223) ([4035f931](https://github.com/kythe/kythe/commit/4035f931711eadd18d81fba7332e6eee83065d4f))
  *  emit columnar callers (#3220) ([e1fe01a6](https://github.com/kythe/kythe/commit/e1fe01a63cae0b9d732323c6a2af0785522eb45f))
  *  allow `write_tables` to compact output LevelDB (#3215) ([2895c1c7](https://github.com/kythe/kythe/commit/2895c1c7d14783edd8dd40d7d87f983a730ae437))
* **sample-web-ui:**
  *  add link to related node definitions (#3227) ([854ab489](https://github.com/kythe/kythe/commit/854ab489cc3f09ddd82d71ba19e9d86f86090a19))
  *  display callers in xrefs panel (#3221) ([5c563dc7](https://github.com/kythe/kythe/commit/5c563dc744f2b7e58511ecfd9ba2ef6190486653))
* **tools:**
  *  entrystream: support aggregating entries into an EntrySet (#3363) ([e1b38f50](https://github.com/kythe/kythe/commit/e1b38f5084b1adc4719d594dee080e6c64084fe3))
  *  kindex: support reading kzip files (#3293) ([2be0c1f0](https://github.com/kythe/kythe/commit/2be0c1f0d1519d24903572b32c42de593eb80449))
* **release:**
  *  add extract_kzip tool to release archive (#3454) ([3ab6111d](https://github.com/kythe/kythe/commit/3ab6111d222cb120a89155ecdc6687c06c0ed7e7))
  *  add protobuf indexer to release (#3358) ([ae537104](https://github.com/kythe/kythe/commit/ae537104b1cbe0fbb6911f534643d209c9c42ed6))

#### Bug Fixes

* **api:**  properly marshal protos with jsonpb (#3424) ([358b4060](https://github.com/kythe/kythe/commit/358b4060eaddb32189fd36073d2ac9f28261a707))
* **cxx_common:**
  *  add enumerator for kDocUri fact (#3378) ([4b8925e0](https://github.com/kythe/kythe/commit/4b8925e0d8ffc5373a2f03c3489e8a9d8c7de1e9))
  *  Add a dependency for strict dependency checking (#3269) ([5a5a230e](https://github.com/kythe/kythe/commit/5a5a230ec48059a9a3203b29b7526fd183752ea1))
* **cxx_extractor:**  segfault when given nonexistent file (#3234) ([6c0fef7a](https://github.com/kythe/kythe/commit/6c0fef7ae8223707f5f546130f22272a1dfc6b7d))
* **cxx_indexer:**
  *  set CompilationUnit.source_file for unpacked input (#3254) ([35706fb1](https://github.com/kythe/kythe/commit/35706fb1d1a526c4b3bc07f35e9e2206a921c20d))
  *  don't crash on empty function/enum bodies (#3281) ([f06d335d](https://github.com/kythe/kythe/commit/f06d335dffca49c17d6d2ff543c9cc4d9fea943c))
* **gotool_extractor:**  when no global corpus is given, use package's corpus for each file (#3290) ([6bc18f57](https://github.com/kythe/kythe/commit/6bc18f5769e6330da3a35afa515fcdcd19cefde2))
* **java_common:**  allow analyzers to throw InterruptedExceptions (#3330) ([01617d9c](https://github.com/kythe/kythe/commit/01617d9cc0753550964c57af57c64644a0808798))
* **java_indexer:**
  *  make workaround source compatible with JDK 11 (#3275) ([7284ac2c](https://github.com/kythe/kythe/commit/7284ac2cb2610b1fa07e3ca4081a9dddbee0f74c))
  *  ensure static import refs are accessible (#3305) ([c453a319](https://github.com/kythe/kythe/commit/c453a319b76da635c7915c8376d0121118c2d8bd))
  *  ensure static import refs are static (#3294) ([0f80fe48](https://github.com/kythe/kythe/commit/0f80fe4816e6e3235113277a616beb9b80701b8c))
  *  do not close System.out after first analysis (#3199) ([e2af342f](https://github.com/kythe/kythe/commit/e2af342f151c090a679f2c6a2d914ba9c724d36e))
* **jvm_indexer:**  prepare code using ASM for Java 11 (#3214) ([94810956](https://github.com/kythe/kythe/commit/948109561829caa91c32378c580ea51192b88b7e))
* **post_processing:**  remove anchors from edge/xref relations (#3198) ([b81ef3af](https://github.com/kythe/kythe/commit/b81ef3afc21ea292c2357faae972ba8cb6b15fc4))

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

[Unreleased] https://github.com/kythe/kythe/compare/v0.0.40...HEAD
[v0.0.40] https://github.com/kythe/kythe/compare/v0.0.39...v0.0.40
[v0.0.39] https://github.com/kythe/kythe/compare/v0.0.38...v0.0.39
[v0.0.38] https://github.com/kythe/kythe/compare/v0.0.37...v0.0.38
[v0.0.37] https://github.com/kythe/kythe/compare/v0.0.36...v0.0.37
[v0.0.36] https://github.com/kythe/kythe/compare/v0.0.35...v0.0.36
[v0.0.35] https://github.com/kythe/kythe/compare/v0.0.34...v0.0.35
[v0.0.34] https://github.com/kythe/kythe/compare/v0.0.33...v0.0.34
[v0.0.33] https://github.com/kythe/kythe/compare/v0.0.32...v0.0.33
[v0.0.32] https://github.com/kythe/kythe/compare/v0.0.31...v0.0.32
[v0.0.31] https://github.com/kythe/kythe/compare/v0.0.30...v0.0.31
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
