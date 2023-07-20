# Release Notes

## [v0.0.63] - 2023-07-18

#### Bug Fixes

* **cxx_indexer:**
  *  don't create a temporary and use its address for usrs (#5720) ([aefac5c6](https://github.com/kythe/kythe/commit/aefac5c66aede4a1a877e7e2644374688331df6a))
  *  ensure the working directory is absolute (#5709) ([df7353e3](https://github.com/kythe/kythe/commit/df7353e3230e97e9266b6f64b59b231503f9072c))
  *  ensure the working directory is absolute (#5708) ([d73735b8](https://github.com/kythe/kythe/commit/d73735b813b12347ae23557b00271d409cd6e299))
* **go_indexer:**  visit anon members in struct type (#5734) ([8bec7289](https://github.com/kythe/kythe/commit/8bec72890e315f5368276163ea5fb6c1e5da05f7))
* **proto_verifier_test:**  remove proto static reflection option (#5702) ([ce9303db](https://github.com/kythe/kythe/commit/ce9303dbbb7aca6b842fdcf5ac8beccfcf70c358))
* **serving:**  scan all identifier matches for reply (#5735) ([47b5f65c](https://github.com/kythe/kythe/commit/47b5f65c7da33fe68a88a86adf8a6ce63d5da231))
* **typescript_indexer:**
  *  emit  edges for methods that are 2+ levels away (#5725) ([580ced87](https://github.com/kythe/kythe/commit/580ced87085d90b07bcbc009aba1770528b7fc03))
  *  improve childof coverage (#5727) ([1ad4c6dd](https://github.com/kythe/kythe/commit/1ad4c6dd42b84521558906f922ffe65a4f8b6c1a))
  *  limit ref/call spans to identifiers (#5695) ([2eedf34f](https://github.com/kythe/kythe/commit/2eedf34f736b9135063cd20b3f7883519f0d2a5a))

#### Features

* **cc_proto_verifier_test:**  optionally enable proto static reflection (#5700) ([d352a0bc](https://github.com/kythe/kythe/commit/d352a0bc881ca1b2188587718f5353ecb08fadd8))
* **cxx_indexer:**
  *  add GraphObserver.recordDiagnostic() (#5730) ([49f57689](https://github.com/kythe/kythe/commit/49f576899d85adfbfe56228bed4ceb14e4c00291))
  *  provide InspectDeclRef access to Expr (#5726) ([d2258d15](https://github.com/kythe/kythe/commit/d2258d15ae4737972fe32ed4cc0f1af9d9898813))
  *  optionally use the compilation unit corpus for USRs (#5710) ([22798b44](https://github.com/kythe/kythe/commit/22798b444a2636cb4917487cd2e92d228d37898d))
* **go_indexer:**
  *  support custom GeneratedCodeInfo edges (#5740) ([79aba518](https://github.com/kythe/kythe/commit/79aba518600520def93a81ad61f663a680a894ab))
  *  add --emit_ref_call_over_identifier flag (#5692) ([8cd5f34e](https://github.com/kythe/kythe/commit/8cd5f34e1c50846c1938f8c4810f2c50146db69e))
* **java_indexer:**  add --emit_ref_call_over_identifier flag (#5693) ([7e74fea3](https://github.com/kythe/kythe/commit/7e74fea3067add289c844ec6bf36affbe9e69101))
* **proto_verifier_test:**  add cc_deps and cc_indexer parameters ([133d3790](https://github.com/kythe/kythe/commit/133d3790d2b8002bbb2013c5d5660e804bcd67ca))
* **typescript:**  add semantic/generated fact name (#5736) ([e1433ba3](https://github.com/kythe/kythe/commit/e1433ba322729830a91cfbd1f9d1c538e6e1034d))

## [v0.0.62] - 2023-06-07

#### Features

*   add InspectFunctionDecl callback to LibrarySupport (#5596) ([5a9af6e3](https://github.com/kythe/kythe/commit/5a9af6e371e0e38d6de86d11b0a4dc8fbf956b26))
*   add KytheURI#toCorpusPath() (#5592) ([eb169e6d](https://github.com/kythe/kythe/commit/eb169e6d60c9167ee9884c172654ff5c3f1c21af))
* **api:**
  *  support returning scopes for xrefs (#5595) ([5e3377ef](https://github.com/kythe/kythe/commit/5e3377ef95a9d617e30a47d4c2cda2dbbfd89053))
  *  also filter directories containing only textless entries (#5582) ([bd0277bb](https://github.com/kythe/kythe/commit/bd0277bbd5dd67cdd749e3ae0ebe3cb6e0d59b01))
  *  guard returning file entries with missing text (#5580) ([3ec5f315](https://github.com/kythe/kythe/commit/3ec5f315c6102b010fadeab6d7675f1cb2abcf27))
* **build:**  add and use permissive wrapper for select.with_or (#5587) ([45379cc0](https://github.com/kythe/kythe/commit/45379cc048123ab3295b2d6b9b8b2b909b667a0e))
* **cli:**
  *  show kind for xref sites (#5607) ([c0b848bb](https://github.com/kythe/kythe/commit/c0b848bb2250df78981c902b09aec45a3a78518c))
  *  add flag for xref semantic scopes (#5602) ([e2de9576](https://github.com/kythe/kythe/commit/e2de95767307403c3c7accc40df5a68b50af91c2))
* **cxx_common:**
  *  optionally dispose of unselected filesets (#5689) ([649d1560](https://github.com/kythe/kythe/commit/649d156089c65b0cd9ba844c57b267eceea49bfd))
  *  use a more compact serialization and in-memory format for selection (#5652) ([2b14f986](https://github.com/kythe/kythe/commit/2b14f986dd8c56a0e3bb7df5fbaec3c86bdca239))
* **cxx_indexer:**
  *  optionally set protobuf aliases as writes (#5636) ([0efe65e1](https://github.com/kythe/kythe/commit/0efe65e1d457bbe3bb89e1b1c1c4741af11cf8f6))
  *  support kTakeAlias semantics in simple alias exprs (#5541) ([5719e42c](https://github.com/kythe/kythe/commit/5719e42c6d0ad9d1328671da4f165eaa6590dae0))
  *  support simple aliases to declrefexprs (#5538) ([b0561485](https://github.com/kythe/kythe/commit/b0561485727cc4f2feee8b0a5f6911e255096834))
  *  add alias-taking semantic (#5540) ([0155c524](https://github.com/kythe/kythe/commit/0155c524ee22944ff4088f4e8cbe66a90ce9a4f0))
  *  prepare to eliminate simple aliases (#5535) ([12de694d](https://github.com/kythe/kythe/commit/12de694db74921b019757baec1f93403e1a0beaa))
* **go_indexer:**
  *  add support for using file as top-level caller (#5637) ([da6a8bdd](https://github.com/kythe/kythe/commit/da6a8bdd96395015b58bfc392816addc359ab528))
  *  render context namespaces recursively in RenderQualifiedName. (#5608) ([bd44dd42](https://github.com/kythe/kythe/commit/bd44dd420fc3713f87a711eb5b6277d084a05148))
  *  support metadata semantic types (#5598) ([e79d8ee9](https://github.com/kythe/kythe/commit/e79d8ee955b63bd1f0a1b12b894b61fdd0773108))
  *  struct refs in composite literals should be writes (#5606) ([b8e27f30](https://github.com/kythe/kythe/commit/b8e27f3080aa13b2d63f8427b959c4cc607de207))
* **go_tools:**  allow sorting units before files on merge (#5562) ([e199abd5](https://github.com/kythe/kythe/commit/e199abd5c93663379d5587c9b5b42d5728444f99))
* **java:**  add semantic/generated=set facts for annotated proto accessors (#5629) ([cfb0fc78](https://github.com/kythe/kythe/commit/cfb0fc78698f757c81be7bb1257538b259a76e47))
* **java_common:**  update shims to more clearly indicated supported version (#5590) ([d418a5d6](https://github.com/kythe/kythe/commit/d418a5d66e7bb01bfb69c990828fd8f81d046e50))
* **java_indexer:**
  *  try to handle unsupported -source flags (#5668) ([5e8ab468](https://github.com/kythe/kythe/commit/5e8ab468bc81f1e92c0a662d08eb48e1b85353ba))
  *  add set semantic to AutoValue setters (#5661) ([8cbd40c1](https://github.com/kythe/kythe/commit/8cbd40c11a5d737fc1ada3fa191e66ab0603e39c))
  *  use file as scope for top-level anchors (#5639) ([f9202daf](https://github.com/kythe/kythe/commit/f9202dafeca6312b8933b561a6e6038ad409f506))
  *  mark this/super ctor calls as direct (#5611) ([656d67bc](https://github.com/kythe/kythe/commit/656d67bcca2f08f0b33b8d2551393f019de50777))
  *  mark ctor calls as direct (#5610) ([7d006726](https://github.com/kythe/kythe/commit/7d006726256e3b495564a38c75dc51d930a39c3a))
  *  add java20 shim for java indexer (#5583) ([bf8a2798](https://github.com/kythe/kythe/commit/bf8a279843aca5302ea1efdd55dbed2d366bc34d))
* **objc_indexer:**  output usr for ObjCMethodDecl (#5564) ([96304cca](https://github.com/kythe/kythe/commit/96304cca6a8d868686cc982711068c9db11e3bbd))
* **schema:**
  *  add semantic/generated fact (#5628) ([4101e315](https://github.com/kythe/kythe/commit/4101e3159ce16240fa5333a8b93c7254c37d6596))
  *  add ref/writes (#5581) ([33ef4d5e](https://github.com/kythe/kythe/commit/33ef4d5eb60117acd6e5f3ab64b893dcf025211c))
* **serving:**  support scoped cross-references (#5530) ([2387903e](https://github.com/kythe/kythe/commit/2387903e66fd9f52346e3234a981e9dd7a74ca82))
* **textproto_indexer:**  mark fields as ref/writes (#5679) ([e30b5e83](https://github.com/kythe/kythe/commit/e30b5e8347f9dacb2a49089048eae3098d69f17a))
* **typescript:**
  *  merge constructor nodes (#5609) ([01de9808](https://github.com/kythe/kythe/commit/01de9808b3163e98b6e8446aef871c590040c07b))
  *  use unique vnames for anonymous functions  (#5593) ([7ef0880b](https://github.com/kythe/kythe/commit/7ef0880ba487004695d46e76d5d4439abc792df7))
  *  emit marked sources for functions, classes and enums (#5578) ([02c7c215](https://github.com/kythe/kythe/commit/02c7c2153dcdeb816784b9c63f0055510a4d7dde))
  *  add ref/id edges for shorthand properties (#5569) ([14d3c9d7](https://github.com/kythe/kythe/commit/14d3c9d7c1236fc5380ee2e0d0e68be140b918fd))
  *  add flag follow aliases when emitting refs (#5563) ([18c2093d](https://github.com/kythe/kythe/commit/18c2093d985c4e062d215a80a35946e2fd4cbdad))
  *  emit code nodes for tsx attributes (#5561) ([91a76289](https://github.com/kythe/kythe/commit/91a76289886a086e05474dc4ec7c071242119269))
  *  launch edge reassignment for import statements (#5560) ([a66de2f9](https://github.com/kythe/kythe/commit/a66de2f9e1ab49f6e8e0537277bcefa8e8883167))
* **typescript_indexer:**
  *  improve marked source CONTEXT nodes (#5642) ([a3904172](https://github.com/kythe/kythe/commit/a390417255ebfcd6c053254cf910e539042792e7))
  *  add flag to emit zero-width spans for module nodes (#5632) ([dfa25327](https://github.com/kythe/kythe/commit/dfa25327cd8185bbb28cedf4360ac89fbabfc008))
  *  handle type nodes when emitting ref/id from object bindings (#5631) ([d282495d](https://github.com/kythe/kythe/commit/d282495deb66df2c79ff96b2a041b6ca125a00e5))
  *  clean up unused options and rename allFiles to rootFiles for clarity (#5533) ([e88d5c69](https://github.com/kythe/kythe/commit/e88d5c694093e2f4c0cc1163043b2af795fbb33a))
  *  add ref/id from constructor calls to class nodes (#5534) ([fbf4be95](https://github.com/kythe/kythe/commit/fbf4be95c8e0078d245e2fb9ac6ee6da47551860))
  *  implement ref inlining for imported symbols (#5527) ([ff8bbd15](https://github.com/kythe/kythe/commit/ff8bbd15e5a34bedfc7a234bd846afe63bcc9018))
  *  support dynamic imports (ref to modules only) (#5514) ([827dc161](https://github.com/kythe/kythe/commit/827dc161ee9c3a6f9aae5dfc749a5ec1c6785fd6))
* **verifier:**
  *  naive implementation of groups for new solver (#5680) ([7e3665d1](https://github.com/kythe/kythe/commit/7e3665d112da401b6f45de65381ec75b690803b1))
  *  add a relation for anchors (#5667) ([30b40393](https://github.com/kythe/kythe/commit/30b40393c13b69a69ae19d853948ac3c0aacc353))
  *  begin supporting some forms (#5665) ([91d7cd04](https://github.com/kythe/kythe/commit/91d7cd04ca03aa45e573fa7dbbd22b44c8a2a6d9))
  *  unknot some deps and call into the fast solver (#5662) ([157e8d1d](https://github.com/kythe/kythe/commit/157e8d1dac67364bd4b5955d8bab2ae0d7ddb489))
  *  generate and run an empty but typed program (#5659) ([67a19fbb](https://github.com/kythe/kythe/commit/67a19fbbe0c2c4155d1ec3d724ffb15bb714abc2))

#### Bug Fixes

*   removal license export (#5634) ([cc8f9c37](https://github.com/kythe/kythe/commit/cc8f9c37466a477813a496a396877f6c66c19fea))
*   fix reversed op (#5599) ([d838ac67](https://github.com/kythe/kythe/commit/d838ac6761c5f9253f0a658d9d71214ff6b4aa99))
* **api:**  explicitly mark source_text optional ∵ File.text is optional (#5666) ([c1a4536b](https://github.com/kythe/kythe/commit/c1a4536b800fbbac2284b8068e3e81495f40000e))
* **cxx_common:**
  *  fix and test minor issue with fileset id 0 (#5655) ([b65dfcf1](https://github.com/kythe/kythe/commit/b65dfcf172c0c2b2249d66106d393ad23b27327f))
  *  select artifacts from failed targets (#5644) ([2bae2d54](https://github.com/kythe/kythe/commit/2bae2d54c55fb05dd1228ba030823a12a91a6ede))
  *  attempt to reduce memory usage of bazel artifact selector (#5638) ([3d737714](https://github.com/kythe/kythe/commit/3d7377146c863d8fe73219292f91a28acc3a4db9))
* **cxx_extractor:**
  *  compile fix when absl::string_view != std::string_view (#5588) ([0389aa8e](https://github.com/kythe/kythe/commit/0389aa8e0dd94aa50e60269c5cec2e5f6403cc55))
  *  don't validate pchs (#5571) ([de71b9cb](https://github.com/kythe/kythe/commit/de71b9cb3943782b622ff1cc694e72881ee6d07e), breaks [#](https://github.com/kythe/kythe/issues/))
  *  don't crash when modules are on (#5554) ([a18df6c8](https://github.com/kythe/kythe/commit/a18df6c88b5cbcd5f812e49e6f9542cd85b8a124))
  *  the main source file should be marked always_process (#5537) ([ad0966ba](https://github.com/kythe/kythe/commit/ad0966ba8320b95dac81591b2b88168ef7dd7895))
* **cxx_indexer:**
  *  mark DesignatedInitExprs as writes (#5616) ([714358f4](https://github.com/kythe/kythe/commit/714358f4652fa64f2188fef9ed35bebf377ef05e))
  *  find vnames for pch/pcm files (const edition) (#5614) ([152f9f67](https://github.com/kythe/kythe/commit/152f9f67898ebf32a7b94962c5944e080434dc51))
  *  find vnames for pch/pcm files (#5612) ([33b2c6e1](https://github.com/kythe/kythe/commit/33b2c6e192b8e5c351fae75130f792d3530b072b))
  *  don't validate pchs (#5573) ([a0393adc](https://github.com/kythe/kythe/commit/a0393adcc58578a1c1e7d0fbce8f506c0dc95d47))
  *  emit proper marked source for macro-renamed functions (#5542) ([89612794](https://github.com/kythe/kythe/commit/896127944be49ab5d6fb83af901fb445145a70bf))
* **cxx_verifier:**  ensure primitive members are always initialized (#5640) ([1625b495](https://github.com/kythe/kythe/commit/1625b495d7554a801512a491e32cdc65279a2e2c))
* **go:**
  *  use %s to format protos in kzip validation error messages (#5674) ([7ed207bf](https://github.com/kythe/kythe/commit/7ed207bff79871081f8bc38bb212c025637530cd))
  *  avoid formatting protos via "%v" for non-debug uses (#5673) ([76110120](https://github.com/kythe/kythe/commit/761101206f1201e8f3d7480b6ec1128186435fc3))
* **go_indexer:**
  *  do not mark map keys as ref/writes (#5641) ([f34d984b](https://github.com/kythe/kythe/commit/f34d984bfee3bd6522f0bb6af499729c064b1352))
  *  don't strip extensions from import paths (#5568) ([ac48a010](https://github.com/kythe/kythe/commit/ac48a010abf2e82bad636457788fdd40676ca54c))
* **java:**  explicitly define toolchains for the Java versions we support (#5615) ([05fe9086](https://github.com/kythe/kythe/commit/05fe9086c8e81de292f541d85bb7d6739e239ee9))
* **presubmit:**  omit manual targets from both input and output sets (#5574) ([d12a0ba3](https://github.com/kythe/kythe/commit/d12a0ba32c0c64f1d53ca394c8683be921d1f58d))
* **proto_indexer:**  use extends relation for proto field extensions (#5521) ([d9213d32](https://github.com/kythe/kythe/commit/d9213d320940392969de9fd7dc8dfca4d4ec509e))
* **serving:**
  *  cleanup/fix crossrefs paging (#5600) ([c2314b7a](https://github.com/kythe/kythe/commit/c2314b7af5f2358e46356a42b6fc39ff8ac6afe2))
  *  don't checklines in diffmatchpatch (#5557) ([10beac0a](https://github.com/kythe/kythe/commit/10beac0a5d8bcb79cf2abe896196d7b7607adf84))
* **tools:**  use vfs.Stat instead of os.Stat in kzip tool (#5566) ([e27ce6b2](https://github.com/kythe/kythe/commit/e27ce6b2d63a8c2e96013b734d845bc8a7c154ae))
* **typescript:**  emit code blocks for interface properties and fix TODO (#5513) ([8efd1273](https://github.com/kythe/kythe/commit/8efd1273c594053935719b5e45152a21f30f9123))
* **typescript_indexer:**
  *  remove console.log left from debugging (#5622) ([185f8e16](https://github.com/kythe/kythe/commit/185f8e16df8df261b72da61b9c986368733116fc))
  *  fix context in marked sources (#5620) ([1e2c50c4](https://github.com/kythe/kythe/commit/1e2c50c4d66a0c7d0222f39ad376049445ac220c))
  *  fix import xref from .ts to .tsx file (#5549) ([a657cc88](https://github.com/kythe/kythe/commit/a657cc88cd4a8f8c4c3611937be21baa626586b0))
* **verifier:**
  *  flat_hash_map + fix spurious warning (#5671) ([eaf9db8b](https://github.com/kythe/kythe/commit/eaf9db8bfc643d092fd9d3571277a3ebef7b15bb))
  *  depend on absl optional library (#5670) ([beb2f6ad](https://github.com/kythe/kythe/commit/beb2f6ad593a24d83f1aa8bbfe9ac87bd94eceea))
  *  parameterize tests over solver (#5645) ([763a65b9](https://github.com/kythe/kythe/commit/763a65b96d23f5136dbfba17b6db817565d75202))
* **xrefs_filter:**  properly merge page lists that represent allPages (#5624) ([6fe9ba39](https://github.com/kythe/kythe/commit/6fe9ba391bba3eecfcc74aa514a54c02e03367d2))

#### Breaking Changes

* **cxx_extractor:**  don't validate pchs (#5571) ([de71b9cb](https://github.com/kythe/kythe/commit/de71b9cb3943782b622ff1cc694e72881ee6d07e), breaks [#](https://github.com/kythe/kythe/issues/))

## [v0.0.61] - 2023-01-19

#### Bug Fixes

*   include TsProject as a typescript rule mnemonic for extraction extra action (#5504) ([a80fd84d](https://github.com/kythe/kythe/commit/a80fd84d1bae5039d94689c1015e0765f1427f86))
*   correctly patch zero-width spans (#5442) ([2f5811d9](https://github.com/kythe/kythe/commit/2f5811d9bac67edaf0999af8f95f3b2f0cd2ec35))
*   passthrough nil Spans without work (#5438) ([a8f3b96e](https://github.com/kythe/kythe/commit/a8f3b96e05c2398493b5253ba38e9f25296a99b0))
*   guard against nil patcher from errors (#5395) ([9a4281bf](https://github.com/kythe/kythe/commit/9a4281bff0daf08bafb8736e1e8b5d657cd666d8))
*   log patcher errors; continue (#5394) ([cbec0f7c](https://github.com/kythe/kythe/commit/cbec0f7c970a9e59c81675f6122ded5550695fb5))
*   update gofmt path in pre-commit docker image (#5376) ([837db591](https://github.com/kythe/kythe/commit/837db5914f54f4377f8a6c50d972b02af8e5d16b))
* **api:**  only count xrefs cut by CorpusPathFilters (#5451) ([781cbf2b](https://github.com/kythe/kythe/commit/781cbf2b930125b3cfc3cfaca439bea81d93c936))
* **cxx:**  use canonical clang include path (#5468) ([8b5bc95c](https://github.com/kythe/kythe/commit/8b5bc95c4cdb9a03a0dfeb13f1e1639f73ab19cd))
* **cxx_common:**  address bug with zero sized files (#5445) ([6a1dcbfc](https://github.com/kythe/kythe/commit/6a1dcbfc3be19d04076869fa1b6280903df3f4e1))
* **cxx_indexer:**
  *  remove unnecessary precondition (#5461) ([1c481fc0](https://github.com/kythe/kythe/commit/1c481fc07b34c4fb0bc3b6915690d8b86df2e7cb))
  *  reintroduce dereferencing to BuildNodeIdForTemplateName (#5383) ([7f6be7a2](https://github.com/kythe/kythe/commit/7f6be7a2de04239c7152d013d75bf210c457a463))
  *  dereference member templates (#5381) ([a57e0fdb](https://github.com/kythe/kythe/commit/a57e0fdbbaa2ae699bfc37bd92d981ee2b4ca3c8))
  *  make logic less tricky and more functional (#5377) ([e8e05c23](https://github.com/kythe/kythe/commit/e8e05c23a8f9f4424d2f53e50a3d9b4f4af2529d))
  *  fix tvar support with aliasing on (#5373) ([12fd89d0](https://github.com/kythe/kythe/commit/12fd89d0b5336084e960f1f6f12fd4ac7509f63b))
  *  fix crash when logging non-identifier names (#5364) ([2ac93dae](https://github.com/kythe/kythe/commit/2ac93dae5ed35e0f79db69774ac5fef6a216ad36))
* **go:**  re-add Compilation struct and associated methods (#5351) ([3b640116](https://github.com/kythe/kythe/commit/3b640116c90bb47c147600d973d68996dc83b675))
* **go_indexer:**
  *  compound assignments should be rw (#5446) ([3f41a10b](https://github.com/kythe/kythe/commit/3f41a10b5449bf763574f38f39f0a89e5ad13a86))
  *  fix go package marked source (#5375) ([2b64f9fd](https://github.com/kythe/kythe/commit/2b64f9fdfae6407922aac022cb2ae192e8bb7aaa))
* **java:**  move runtime dependencies to runtime_deps (#5433) ([29037261](https://github.com/kythe/kythe/commit/290372617db4dce660b0b049161fa1c7187a6079))
* **java_extractor:**  disable service processors when extracting from Bazel (#5368) ([1920ad60](https://github.com/kythe/kythe/commit/1920ad607cfe6ad65acf1e6410dcddba09798245))
* **java_indexer:**
  *  properly handle null parent trees (#5448) ([e0c175ab](https://github.com/kythe/kythe/commit/e0c175abd566ba956c935635b49f924333635cda))
  *  return null instead of throwing for unsupported attribute views (#5441) ([e5d3e384](https://github.com/kythe/kythe/commit/e5d3e384261303563fde214338570a6e5d8e66c4))
  *  fix marked source test in JDK19 (#5435) ([ba71f5f2](https://github.com/kythe/kythe/commit/ba71f5f2c2316e71fc6f8cfe4835b0309443bdc3))
  *  assign all diagnostics a corpus (#5337) ([024aba95](https://github.com/kythe/kythe/commit/024aba95140c497bb19ef87f884b090f6f900e89))
* **proto_indexer:**
  *  only decorate the type name in service definitions (#5478) ([272cac30](https://github.com/kythe/kythe/commit/272cac301772fbea67ea79158ff06181555c6743))
  *  properly handle comments that end with an empty commented line (#5427) ([e979b1e3](https://github.com/kythe/kythe/commit/e979b1e37b2839c698517a72df4b361d6a1a6e5a))
* **rust_common:**
  *  migrate from tempdir crate to tempfile (#5489) ([0fa54f72](https://github.com/kythe/kythe/commit/0fa54f72c7a715626817eab71e9d8a8727b69965))
  *  fix compatibility with 2022-12-21 (#5482) ([8d422877](https://github.com/kythe/kythe/commit/8d422877d34faa4323c53902868e7920bb630f51))
  *  fix clippy lints of new rust version (#5425) ([d746a506](https://github.com/kythe/kythe/commit/d746a506e5276b945c2b64314bc9f0cbf85c3485))
  *  fix rustc_lib location in release (#5339) ([5036c159](https://github.com/kythe/kythe/commit/5036c159387d25eb2c0e3b564b19ebde39b873c7))
* **rust_extractor:**
  *  ensure glob result is a file (#5470) ([8e023633](https://github.com/kythe/kythe/commit/8e023633326c6b73ba8941e8ac2e02e30eb9fab7))
  *  set extractor_test script as executable (#5382) ([329663bf](https://github.com/kythe/kythe/commit/329663bf4c75d21053f235f1690b02cd9ea81df8))
  *  improve logic for finding the save analysis file (#5354) ([c3bae69e](https://github.com/kythe/kythe/commit/c3bae69e2c85a1649c8dd508ef7ce7183636d814))
* **rust_indexer:**  set root when creating vname for reference (#5471) ([f87b41e7](https://github.com/kythe/kythe/commit/f87b41e7165766b0997e53d8e238edc0e7f068d3))
* **schema:**  get rid of more abs stuff (#5420) (#5439) ([dfb439a6](https://github.com/kythe/kythe/commit/dfb439a63de909e9eb67d0f36fc2471fe0693afb))
* **serving:**
  *  only count unskipped pages (#5477) ([2dbe8785](https://github.com/kythe/kythe/commit/2dbe878584f4b56fa7b278cf36003ebe6a672753))
  *  ensure page read ahead stops promptly (#5473) ([005a6fb6](https://github.com/kythe/kythe/commit/005a6fb68395a3059bd52a0c557831fdb22ecc4c))
  *  clear empty xref sets after all reads (#5465) ([48cd9d01](https://github.com/kythe/kythe/commit/48cd9d0106ca2ae0b62c5d0e33cd951a1ff7417a))
  *  fix reversed inequality (#5464) ([1b69b531](https://github.com/kythe/kythe/commit/1b69b531bcca46218afbb91bb37c1aefd732ac05))
  *  skip PageSet check when given no filters (#5463) ([207c04ff](https://github.com/kythe/kythe/commit/207c04ff17255442f30bf2fe94da47994de7b414))
* **textproto:**  use ref/file for proto-file schema comments (#5416) ([e055c8a0](https://github.com/kythe/kythe/commit/e055c8a0ede82cac870de29d3be938bc9af3ae23))
* **typescript:**
  *  update typescript dependency to 4.8.4, move to apsect-build/rules_ts (#5495) ([301a10ab](https://github.com/kythe/kythe/commit/301a10abb34731951baa67d846afa51ccdb099be))
  *  emit code/json not code facts; remove proto dependency (#5494) ([2996b793](https://github.com/kythe/kythe/commit/2996b79347147b46cfe2b2f1617d73dced9a6365))
  *  add knob to disable some tests (#5479) ([dddd1dd2](https://github.com/kythe/kythe/commit/dddd1dd27e4c447ce79b7ccdc09b84eec958a9f2))
* **typescript_indexer:**  fix linting issues (#5455) ([b5af7ecf](https://github.com/kythe/kythe/commit/b5af7ecf868bfa0a487556699300ee1021634b8b))

#### Performance

*   update patcher to use binary search (#5434) ([dda61f6f](https://github.com/kythe/kythe/commit/dda61f6f6753487c60087abdf518351a5c553c22))
* **serving:**
  *  only filter groups that match requested kinds (#5454) ([519da956](https://github.com/kythe/kythe/commit/519da9561dfd8ff3d669134f7419132c0f225498))
  *  reduce patching to offsets; drop text (#5422) ([69bef08d](https://github.com/kythe/kythe/commit/69bef08de731d0a8e7aecdb54f2b9a97a4d2dbe7))

#### Features

* **api:**  add filtered xrefs counts to reply (#5447) ([4035fe45](https://github.com/kythe/kythe/commit/4035fe4546486eab77b6059dfefac6b892b210f6))
* **cxx:**  emit member visibility facts (#5467) ([9f6d6847](https://github.com/kythe/kythe/commit/9f6d6847ddf644602f71eb6f2a03416628e3a649))
* **cxx_indexer:**
  *  distinguish direct function calls (#5378) ([134d58a5](https://github.com/kythe/kythe/commit/134d58a56f145458d4341a3e58e89ae1ca84e0d7))
  *  handle deprecation via clang::availability (#5365) ([21d416c7](https://github.com/kythe/kythe/commit/21d416c73e94cc07704170f8fff37a25275fe99b))
* **go_indexer:**
  *  implement ref/writes edges for members (#5399) ([50e7e238](https://github.com/kythe/kythe/commit/50e7e23872ef10689ea67042ec741c65338bd670))
  *  implement ref/writes edges (#5398) ([6c8237dc](https://github.com/kythe/kythe/commit/6c8237dc6d97dedb30910cd47ea1a379c37a6b91))
  *  rework --use_compilation_corpus_as_default (#5338) ([9a4050a3](https://github.com/kythe/kythe/commit/9a4050a346f68533310be4b9466829c894ee4cca))
* **go_serving:**  add ResolveVName to Unit interface (#5474) ([90ee7a3c](https://github.com/kythe/kythe/commit/90ee7a3c878a28bf5dc477f005668dd4f74b5b8f))
* **indexing:**
  *  support rewriting JSON-encoded code facts in proxy (#5491) ([c8359d19](https://github.com/kythe/kythe/commit/c8359d19d66a78db47465f0c94bad8dfe872a90e))
  *  allow an AnalysisResult return from the driver (#5481) ([872432aa](https://github.com/kythe/kythe/commit/872432aac6acda1b00867a20f05b9446cbdbaa00))
  *  add details field for AnalysisResults (#5480) ([6f17a714](https://github.com/kythe/kythe/commit/6f17a71442b54c155362b25f7092605a5e7d3be3))
* **java_common:**
  *  improve JDK19 source compatibility (#5432) ([ce8e3b13](https://github.com/kythe/kythe/commit/ce8e3b133aa1afcd471c152e20d8638cdcc41454))
  *  include reflective fallback for compatibility shims (#5428) ([dba039ca](https://github.com/kythe/kythe/commit/dba039ca4fdbef9483687ffca05822da535d25ac))
  *  add function to render signature as plaintext (#5400) ([a70ccc01](https://github.com/kythe/kythe/commit/a70ccc016d38d6d3406e6f4638294092265325f2))
* **java_indexer:**  implement ref/writes edges (#5423) ([b426d6db](https://github.com/kythe/kythe/commit/b426d6db0fc40d1bfa6a7829460e853de9741a92))
* **schema:**
  *  document ref/writes edges (#5452) ([cfe09e00](https://github.com/kythe/kythe/commit/cfe09e0096ac3aeec98e4f5c2918b2386fff2c7a))
  *  document canonical order of schema tags (#5440) ([7c9bf1b0](https://github.com/kythe/kythe/commit/7c9bf1b0bff8507003becfa60c89c15bf527f69f))
* **serving:**
  *  add option to read xref pages concurrently (#5469) ([9493a8d4](https://github.com/kythe/kythe/commit/9493a8d4ac6ff23f259bd46a6aae2713cbcee5a9))
  *  allow some leeway time to return before xrefs deadline (#5460) ([a9cc5003](https://github.com/kythe/kythe/commit/a9cc5003712f1493b1821cd4665106ad9cfc2905))
  *  trigram search index for xref pages (#5456) ([78c7db93](https://github.com/kythe/kythe/commit/78c7db93f14e021c60ce3d8ce72e35e5ed8fb2a2))
  *  support marshalling Patchers (#5424) ([54a8664a](https://github.com/kythe/kythe/commit/54a8664a5191c2d3dee830480c2919bb519d11ed))
* **tooling:**
  *  mark conflicting /kythe/code facts as ok with ignore flag and add tests (#5410) ([73dab3e8](https://github.com/kythe/kythe/commit/73dab3e8fda62194c6fd8396ffd731a5de6edaa2))
  *  add a flag to ignore conflicting /kythe/code facts (#5409) ([e99afd28](https://github.com/kythe/kythe/commit/e99afd280522ba800fd4fbc31b9099093ef8a435))
* **typescript_indexer:**  add support for ref/writes (#5444) ([9d5400d2](https://github.com/kythe/kythe/commit/9d5400d230812949b0d30a3b566732737f2673f1))
* **verifier:**  support code/json fact (#5493) ([0a9814e9](https://github.com/kythe/kythe/commit/0a9814e9252dc451d961d555a57f9fad80375d16))

## [v0.0.60] - 2022-08-01

#### Bug Fixes

* **build:**  remove use of managed_directories (issue #5287) (#5288) ([8ad0e685](https://github.com/kythe/kythe/commit/8ad0e685616462dbf85472984c99aa5d2ea6de3f))
* **cxx_indexer:**  emit non-implicit refs to explicit template specializations (#5290) ([9f0a3379](https://github.com/kythe/kythe/commit/9f0a33793c624043529cf5c8510c79587423d93f))
* **java:**  rollback recent annotation ref/id changes (#5294) ([77a9a8e6](https://github.com/kythe/kythe/commit/77a9a8e6d3db4d24be19d31632651e44cd49908a))
* **java_indexer:**
  *  guard against NPE in ImmutableList (#5310) ([74dc8410](https://github.com/kythe/kythe/commit/74dc8410d24586bccb1f7853a8f37019fcba6e46))
  *  assert that annotation invocations aren't refs (#5284) ([4b3e0f44](https://github.com/kythe/kythe/commit/4b3e0f44ac11b908e3299dd24c34ff5dfc00c373))
* **rust_extractor:**  find the analysis file based on crate name (#5306) ([48b4edf3](https://github.com/kythe/kythe/commit/48b4edf36193ffe9f1b8d5263c54e6e2d058f6e4))
* **rust_indexer:**
  *  set the VName root on emitted Rust anchors (#5329) ([d282719f](https://github.com/kythe/kythe/commit/d282719f5fe1d97ff260964335f59145ed927a3e))
  *  clean relative paths in indexer to match vname paths (#5279) ([0cd64758](https://github.com/kythe/kythe/commit/0cd64758cd54ff567bbe9babf74d4364a02c6859))
  *  set corpus+lang on diagnostic (#5263) ([7274df4c](https://github.com/kythe/kythe/commit/7274df4cc8c09d5cbf0f2c6123cbcd8fa836d5c1))
* **textproto:**  workaround upstream proto path bug (#5307) ([e770a7b0](https://github.com/kythe/kythe/commit/e770a7b03fb44ed5e2a3889aae10c2fbda77f734))
* **typescript_indexer:**
  * fix lint issue (#5265) ([56a3e50d](https://github.com/kythe/kythe/commit/56a3e50de0bdaab5892d614f0ba2cd66178b3d59))
  * don't emit absvar nodes (#5273) ([1e4467b1](https://github.com/kythe/kythe/commit/1e4467b1051820340b90b510a14d68d079b2d64f))
* **verifier:**  include @ in anchor labels and support EVars (#5291) ([992a3e94](https://github.com/kythe/kythe/commit/992a3e946df73948afaba06e384b1d164e27a86f))

#### Features

* **api:**  add resolved path filters for xrefs (#5274) ([cdde24e7](https://github.com/kythe/kythe/commit/cdde24e79b7b93670e9f89479942178bd30bfa27))
* **cxx_indexer:**
  *  record full signatures in a separate file (#5323) ([179c0282](https://github.com/kythe/kythe/commit/179c0282dc82000182cae84dbdcb08650345ad55))
  *  implement directory traversal in indexer VFS (#5325) ([541aa64b](https://github.com/kythe/kythe/commit/541aa64bf3e5831c165a14b2da77e73b629c9f97))
* **go_indexer:**
  *  add --override_stdlib_corpus flag (#5314) ([5617eb8f](https://github.com/kythe/kythe/commit/5617eb8f211b0ee7f421f299f6c3b041e39c54e8))
  *  optionally put builtins and stdlib items in main corpus (#5124) ([f83f9321](https://github.com/kythe/kythe/commit/f83f9321570d81a95039751278d41c5c198cf918))
* **java extractor:**
  * use default value if corpus is ambiguous (#5305) ([40808f97](https://github.com/kythe/kythe/commit/40808f97d474b2e6a609b585974311c4b8d32892))
  * assign a corpus to compilation units (#5320) ([aa76256b](https://github.com/kythe/kythe/commit/aa76256b6fc2818301074f8225e646481669608c))
* **java_indexer:**  
  * use ref/id in annotation utterance contexts (#5281) ([584f65e7](https://github.com/kythe/kythe/commit/584f65e74a2bb016add70f609b310c027c78876c))
  * log error when CU VName has empty corpus (#5303) ([c22d736b](https://github.com/kythe/kythe/commit/c22d736be1693aa43c9e85040fa1bc0d3934b585))
* **proto:**  add a proto field to signify whether a file is protected (#5308) ([d08fbe6b](https://github.com/kythe/kythe/commit/d08fbe6b8af92efcb8639421362c71447e262c68))
* **rust_indexer:**  emit proper xrefs to files generated by dependencies (#5326) ([82a61189](https://github.com/kythe/kythe/commit/82a61189a1af2558a3cec703f3a9a9e003ee7557))
* **tooling:**
  *  add --workspace_uri to kythe decor cmd (#5313) ([18f24c21](https://github.com/kythe/kythe/commit/18f24c21fb787dfb5de60d94f5934b1cbd26d287))
  *  treat absolute kzips as a set (#5271) ([b5e81531](https://github.com/kythe/kythe/commit/b5e8153151dbe3a2c8a352f10c4a9081f03c375e))
* **typescript_indexer:**
  *  emit `field` subkind fact for class/interface/ojbect literal properties (#5293) ([3a3c772b](https://github.com/kythe/kythe/commit/3a3c772b4b9dc91692d961980d70943ef91700f9))
  *  use ref/id for destructuring variables (#5292) ([60c0e854](https://github.com/kythe/kythe/commit/60c0e854e860314e9201dd008b53f1d20ad99c60))
  *  emit edges from object literals to corresponding types (#5262) ([c496c5c0](https://github.com/kythe/kythe/commit/c496c5c00177e437e28bb3a75a5ef811d5d55ff7))
* **verifier:**
  *  allow simplifying graph output data (#5286) ([b6554980](https://github.com/kythe/kythe/commit/b655498061af8866ac2822c8beedfa0a043a6f09))
  *  add a flag to elide nodes not bound as goals (#5269) ([2d4bc69e](https://github.com/kythe/kythe/commit/2d4bc69eaf31016bc86d53e2bd1b48ac7b70adc9))

## [v0.0.59] - 2022-04-18

#### Features

* **api:**
  *  support patching Documentation definitions (#5244) ([fa5b6aa9](https://github.com/kythe/kythe/commit/fa5b6aa9e0e6adbbd5601acf2ba8121c7c0841b8))
  *  support filtering xrefs by their enclosing files (#5242) ([bf5b1ac0](https://github.com/kythe/kythe/commit/bf5b1ac037da8d3edca3739ba664f127d5270a57))
  *  support patching FileDecorations definitions (#5241) ([44cece43](https://github.com/kythe/kythe/commit/44cece4393a4fe1cfd58e0d4786a4f753cb3cbd3))
  *  allow a Workspace to be passed for xrefs patching (#5224) ([38c43a11](https://github.com/kythe/kythe/commit/38c43a11c439250fb2fae5c8a0e0885b0521fbc3))
* **go_indexer:**  populate ReleaseTags/ToolTags with defaults (#5237) ([0ad2c8ae](https://github.com/kythe/kythe/commit/0ad2c8aec9eebe61fe7b64662d69071dd789d736))
* **java:**  bump Java version to 11 (#5223) ([f4af9da3](https://github.com/kythe/kythe/commit/f4af9da356678f80ee453b47c83967afe0638e32))
* **schema:**  add flag nodes (#5243) ([e72c520a](https://github.com/kythe/kythe/commit/e72c520a107c974a0c5d4d48759f1409c1ee3bf4))

#### Bug Fixes

* **cxx_indexer:**  remove another source of corpusless nodes (#5247) ([33fb1f0d](https://github.com/kythe/kythe/commit/33fb1f0dc43803ac32915198567330f70dfa198f))
* **doc:**  update description and examples for extends edge kind (#5229) ([be6d3fd4](https://github.com/kythe/kythe/commit/be6d3fd4a37867837519af6259453e9bacc41edf))
* **extraction:**  fix javac9 extractor path (#5234) ([e57ba0a6](https://github.com/kythe/kythe/commit/e57ba0a6000d10d23ed6d6ffb16c5b484699eb58))
* **go_extractor:**  handle top-level source files in modules (#5239) ([888c26d9](https://github.com/kythe/kythe/commit/888c26d9c4d3d98a8c0d4881271437efc5e2ec66))
* **go_indexer:**  fix satisfies check for 1.18beta2 (#5235) ([60bd2fad](https://github.com/kythe/kythe/commit/60bd2fad0ecd8d6b8eb2f4bbb3b8e3117a74ce0e))
* **java:**  emit wildcard generics in default corpus (#5210) ([e8886d54](https://github.com/kythe/kythe/commit/e8886d540bd64b42ea4e29f396eaae7e18dbea2b))
* **serving:**  close MultiFilePatcher when requests exit early (#5245) ([26df4b61](https://github.com/kythe/kythe/commit/26df4b6119d4248b129983a8304a44c0b6250084))

## [v0.0.58] - 2022-02-22

#### Deprecation

* **java:** last release with JDK 8 support

#### Bug Fixes

* **cxx_indexer:**
  *  constructer initializers should be writes (#5220) ([9efa5c92](https://github.com/kythe/kythe/commit/9efa5c92014021c99ede023c912465a1cee6845e))
  *  don't traverse field decls in lambdas (#5215) ([ae62778b](https://github.com/kythe/kythe/commit/ae62778be330a963078c55226f41008c56a01d9b))
  *  use function names, not exps, for semantics (#5213) ([a11bf38f](https://github.com/kythe/kythe/commit/a11bf38fdb72ae0362b042a1917639f7b5ab8e28))
  *  treat calls to inherited constructors like other members (#5198) ([3db22d38](https://github.com/kythe/kythe/commit/3db22d38823f1edb6cec1204448f50c7e086e1bf))
* **go extractor image:**  updates for extracting to a single corpus (#5173) ([8697a6c4](https://github.com/kythe/kythe/commit/8697a6c4b15caf12b59d8e32b9ac0ee4aa9306bc))
* **go_indexer:**
  *  fix refs to anonymous members across packages (#5195) ([e40b0a0c](https://github.com/kythe/kythe/commit/e40b0a0c1ffa4e8a67199b5e147b097dd5414aca))
  *  ensure generic references work across packages (#5166) ([f2701611](https://github.com/kythe/kythe/commit/f27016111e0a19c6f9628e4b106aa3eff9c0e14e))
* **java:**
  *  do not emit doc nodes for non-javadoc commits by default (#5191) ([b3b97642](https://github.com/kythe/kythe/commit/b3b976429254d979b739a304f98f0fb5152a180b))
* **java_indexer:**
  *  use ref/id edges for class references in new expressions (#5211) ([e151c464](https://github.com/kythe/kythe/commit/e151c46460e7991c32dc0fd8bac24326219a8238))
  *  set the corpus path of diagnostics (#5203) ([cf72c6e8](https://github.com/kythe/kythe/commit/cf72c6e8595dee132a02291464744b8d49da91e4))
  *  fix ambiguous variable def position (#5196) ([159055e4](https://github.com/kythe/kythe/commit/159055e43fb1ded7bcf14cb7f3e9fdf098415d70))
* **proto_indexer:**
  *  actually elide refs for top-level map types (#5219) ([84181c2a](https://github.com/kythe/kythe/commit/84181c2afb926ee4d168caecab0fea97aa4695a7))
  *  elide references to builtins and maps (#5212) ([0501c36b](https://github.com/kythe/kythe/commit/0501c36b8f1bbae2391a7df5b56361bd2d9601a4))
* **rust_common:**
  *  update Rust toolchain to fix tests on macOS Monterey (#5183) ([86dbcbb8](https://github.com/kythe/kythe/commit/86dbcbb8d765a13476741dc9cfa0479e557f72da))
  *  clippy fixes for new Rust version (#5161) ([d65a6871](https://github.com/kythe/kythe/commit/d65a6871612136cb190c623f67351ff5466540fa))
  *  ensure release rust extractor script is executable (#5155) ([64969a85](https://github.com/kythe/kythe/commit/64969a853711c228b4e3cfc3ce91b84b5bb853d7))
* **rust_extractor:**  set the working_directory (#5200) ([07f4e5e5](https://github.com/kythe/kythe/commit/07f4e5e51239f6c4d3364be8bdc7ce711a9b6879))
* **rust_indexer:**  emit built-in types in the same corpus as the CU (#5202) ([63bc29e3](https://github.com/kythe/kythe/commit/63bc29e3ba35fee7d1004948c043e4123dd905ac))
* **serving:**  do not unnecessarily read indirection pages (#5178) ([4b9c4789](https://github.com/kythe/kythe/commit/4b9c4789d596f659c087dd09028ec7541cbccbf4))

#### Features

* **cxx_indexer:**
  *  handle more proto functions (#5209) ([effb97ae](https://github.com/kythe/kythe/commit/effb97ae9d6ee0f97c36af7ad4c76acac08acb2d))
  *  guess protobuf semantics (#5188) ([7ddc015f](https://github.com/kythe/kythe/commit/7ddc015f56c6f2f827df864580bce9b69e81835b))
  *  support r/w and influence for user-defined operators (#5187) ([c3236f7d](https://github.com/kythe/kythe/commit/c3236f7dc0201f5dfb01fa11a8f0d643785ec59e))
* **go extractor:**  add flag to put deps in default corpus (#5169) ([234f17f6](https://github.com/kythe/kythe/commit/234f17f6fa606f0aa7f8e5e82be71f1ed0cf32fd))
* **go_indexer:**
  *  add definition links to MarkedSource (#5194) ([86948564](https://github.com/kythe/kythe/commit/86948564a1002b5531a1e4df91939abae32c07c3))
  *  correct instantiated member references (#5163) ([fccf5440](https://github.com/kythe/kythe/commit/fccf54400fe1ce438d8388c38464ccd907cd7484))
  *  add receiver type parameters to MarkedSource (#5160) ([19e1699c](https://github.com/kythe/kythe/commit/19e1699cfd6efa343767b7732128e3bb2e317a6e))
  *  add MarkedSource for function type parameters (#5159) ([9bad9c81](https://github.com/kythe/kythe/commit/9bad9c81e24fb6c33cd7d89c3b8e1d5dcdce22a6))
  *  emit tapps for instantiated Named types (#5158) ([dd574ee5](https://github.com/kythe/kythe/commit/dd574ee523b12d5d16a316d3c7939a8680c0b220))
  *  ensure references are to non-instantiated methods (#5157) ([7dc3b79f](https://github.com/kythe/kythe/commit/7dc3b79fea2a7709b0aac69403157ea60ee294cd))
  *  support satisfies edges for instance types (#5156) ([eed1ece4](https://github.com/kythe/kythe/commit/eed1ece4eb42eddb97def548ed1592b13249c343))
* **rust_indexer:**
  *  add flag to disable emitting xrefs for stdlib (#5197) ([1aa5b943](https://github.com/kythe/kythe/commit/1aa5b9437c1c3f9fbb9d6d5e742eaee2b0c121f3))
  *  remove need for save-analysis files to be saved to fs (#5193) ([3806d62d](https://github.com/kythe/kythe/commit/3806d62d7cfeab9fa0f06a5c2023dfd356307009))
* **schema:**  include Go generics examples (#5154) ([4b7ae7b2](https://github.com/kythe/kythe/commit/4b7ae7b2fb2213a5fc04ab029314dc43219cc0b3))
* **serving:**  add Hash to FileInfo protos (#5207) ([dd53bcde](https://github.com/kythe/kythe/commit/dd53bcdec65fa35591d8d650e801aa4e9e60ef44))
* **tooling:**  Support filtering of kzips by language. (#5167) ([d3cea029](https://github.com/kythe/kythe/commit/d3cea0294253a6c7e5d6fae7471b09c007a583bf))

## [v0.0.57] - 2021-12-16

#### Bug Fixes

* **java:** Update Flogger dependency to 0.7.3 due to log4j (#5152) ([9a9fb44](https://github.com/kythe/kythe/commit/9a9fb44c710dc3dabf1309840f20790a7325fc83)))

## [v0.0.56] - 2021-12-13

#### Bug Fixes

* **java:** Update Flogger dependency to 0.7.2 due to log4j (#5147) ([ec0359e](https://github.com/kythe/kythe/commit/ec0359e92f253e6e412cb063539eaf4e571ef6a2)))

## [v0.0.55] - 2021-10-18

* Fixes an issue with the Bazel Rust extractor

## [v0.0.54] - 2021-10-04

fuchsia_extractor has been updated to no longer include .rlib files

## [v0.0.53] - 2021-08-27

#### Bug Fixes

*   style nit in the TS indexer (#5044) ([2f1f2a37](https://github.com/kythe/kythe/commit/2f1f2a3763dd02c841c70f16234f6a2907f71e16))
*   Added correct underline syntax (#4967) ([733a6cf6](https://github.com/kythe/kythe/commit/733a6cf674bda52660d5c2354ea6dc771a07a458))
*   update link to re2 syntax (#4952) ([e8808410](https://github.com/kythe/kythe/commit/e88084101133c9810ec28e399022fb4053a71f4c))
*   force selection of `ar` to unbreak os x build of libffi (#4905) ([99ccb48d](https://github.com/kythe/kythe/commit/99ccb48dc4e7aec2fa6a4286d823755952719bf0))
* **build:**  move BUILD rules next to go code (#5009) ([3dc332be](https://github.com/kythe/kythe/commit/3dc332be9b254ddd34de4ac2051d11bd5ec59dd3))
* **cc_indexer_test:**  use short_path for generated files (#5036) ([da29f512](https://github.com/kythe/kythe/commit/da29f512cc4e8920a565e6006a91338cbc69cb51))
* **cxx_indexer:**
  *  fix dangling reference (#5039) ([8c0140b4](https://github.com/kythe/kythe/commit/8c0140b45b161c4f9f603adc306bc974d59c4540))
  *  assign usr nodes to the empty corpus (#4928) ([12f483af](https://github.com/kythe/kythe/commit/12f483af42203c83dea45effaf27378945281f78))
  *  refactor lvalue handling for dataflow (#4907) ([9035307b](https://github.com/kythe/kythe/commit/9035307bd5090505e8a6aecc527abf5baeebf870))
* **docs:**
  *  include a note about anchor-to-file childof edges (#5007) ([98391d0a](https://github.com/kythe/kythe/commit/98391d0af65e4bbbb018afe030a319453ba5672f))
  *  use wrapper script for extracting make-based projects (#4997) ([1df8f54c](https://github.com/kythe/kythe/commit/1df8f54cc9b56ee7cf6c197e7e3aca08387a4554))
  *  better highlight macos link, remove bazelisk recommendation (#4939) ([dee76d77](https://github.com/kythe/kythe/commit/dee76d776b88f9c7bb74d95c7eab66809e8de166))
* **fuschia-extractor:**  Fix panic format string for Rust 2021 (#4958) ([5edce0e5](https://github.com/kythe/kythe/commit/5edce0e54e3406d0a69a2fdbedc701d7a66e2691))
* **go_indexer:**  check whether embedded type is Named (#5028) ([7babd407](https://github.com/kythe/kythe/commit/7babd407db6e5e6918634df063cd972f4240fc54))
* **java:**
  *  guard against possible null sym field (#4892) ([b5e52f15](https://github.com/kythe/kythe/commit/b5e52f1555198d5796bde2f31fea576c82f9fad5))
  *  properly elide type arguments for constructor calls (#4889) ([160b28af](https://github.com/kythe/kythe/commit/160b28af24a477fd75458e033523cf9ce81733ee))
  *  verifier test exposing the compiler implicitly tagging enum values as "static" (#4887) ([9b4aace9](https://github.com/kythe/kythe/commit/9b4aace96d66d76a075d9fc0ef850c7610d1a545))
* **java indexer:**  --override_jdk_corpus takes precedence over default corpus (#5043) ([08aea04e](https://github.com/kythe/kythe/commit/08aea04e05c29bf4541ca419ebe1013fe5cb4ef2))
* **java_extractor:**  properly filter/normalize Bazel javacopts (#4977) ([59a94bf7](https://github.com/kythe/kythe/commit/59a94bf76c016a6d13a6d41d9f3b8e31de178a0f))
* **kzip info:**  allow paths that begin with /kythe_builtins/ (#5030) ([7a909d91](https://github.com/kythe/kythe/commit/7a909d91690893fdd5c3dc09dfcf4163cdac61e5))
* **rust_common:**
  *  remove reference to PROTO_COMPILE_DEPS (#5018) ([bbaf4eb6](https://github.com/kythe/kythe/commit/bbaf4eb6d20c450cfa7dcee95c884dedf8b7d983))
  *  remove references to PROTO_COMPILE_DEPS (#4992) ([efa7141f](https://github.com/kythe/kythe/commit/efa7141fec3019cd743e635eb80c994700924b50))
* **rust_extractor:**
  *  process env vars and compiler args passed to wrapper (#5014) ([d315b5be](https://github.com/kythe/kythe/commit/d315b5be015f7a62ef68b35e287f60a8219653b8))
  *  fix running the Rust extractor on macOS (#5013) ([5db173cd](https://github.com/kythe/kythe/commit/5db173cd66773704f9a6f4ba8eb6a4c7f8bd332a))
  *  make extractor tests more resilient (#5002) ([6f719653](https://github.com/kythe/kythe/commit/6f719653e8b5d6c61d88f6c62ae3612d3f92605d))
  *  change argument processing due to rules_rust change (#4996) ([3044115c](https://github.com/kythe/kythe/commit/3044115c9a2c319d85358a0d98d30205a90f90c7))
  *  fix --out-dir path (#4993) ([c1afdc7a](https://github.com/kythe/kythe/commit/c1afdc7abdc4a66a637d06cd146f7150baa90b16))
  *  Remove assert_cmd dependency from integration test (#4972) ([f36068ae](https://github.com/kythe/kythe/commit/f36068aec92977ecf2c591c1749cb5ddd288c4d6))
* **rust_indexer:**
  *  properly handle empty file contents (#5034) ([b89f1678](https://github.com/kythe/kythe/commit/b89f167819a745472376cd5bc8a1fa7fe0a524a9))
  *  Fix proxy indexer (#5032) ([d1ccce08](https://github.com/kythe/kythe/commit/d1ccce08e895d481d47fb1f0a175994ea174d7bb))
  *  allow files to be passed in for the extractor (#5024) ([65ec646d](https://github.com/kythe/kythe/commit/65ec646d2a65578d856e4de9b6543753984dd291))
  *  remove reference to go_verifier_test (#5020) ([8dd99679](https://github.com/kythe/kythe/commit/8dd99679e7a96f821ca3d48bd9ff7a63d096a865))
  *  copy all generated proto files (#4968) ([1e199931](https://github.com/kythe/kythe/commit/1e199931015b5a37d371041df950c6002b6c4945))
  *  rustfmt everything (#4941) ([82504c0a](https://github.com/kythe/kythe/commit/82504c0a51ddc77ffc29e1e37a3d11edce980b43))
  *  update to a recent nightly toolchain (#4940) ([e77b26d4](https://github.com/kythe/kythe/commit/e77b26d4c74bb42d8b13851cec9698d492c1c8e8))
* **tool:**  add alias support to kythe command, add alias for "decor" (#4937) ([9fe7e8ea](https://github.com/kythe/kythe/commit/9fe7e8ea7da05eb074581ac988cf117d10c346e7))
* **typescript_indexer:**  fix compilation issues with TypeScript 4.3 (#4945) ([828d622c](https://github.com/kythe/kythe/commit/828d622c024358e6de4b84e82757299e66536a6e))
* **verifier:**  remove darwin stubs and alwayslink libffi to satisfy dyld (#4906) ([795ce805](https://github.com/kythe/kythe/commit/795ce80580f292863e9ac9ef94dadec77414669e))
* **vscode:**  update vscode dependencies (#4920) ([ed78ca68](https://github.com/kythe/kythe/commit/ed78ca68d82a9b2d512f1f6c9a8fe2938567889b))

#### Features

* **api:**  add set of languages appearing in Origin (#4947) ([d5419f31](https://github.com/kythe/kythe/commit/d5419f314ab4e9c6d72096c8714751066e580327))
* **build:**
  *  add image and cloudbuild configs for pre-commit (#4898) ([63fa149b](https://github.com/kythe/kythe/commit/63fa149b5d4e5cb9c8373a60a4eb8a0403ca8a52))
  *  include a custom bazelrc for cloud builder (#4897) ([b0c64e1d](https://github.com/kythe/kythe/commit/b0c64e1dd7a5040086253fef1725a9c5afa300d5))
  *  add cloud build configuration for presubmit tests (#4894) ([407fa2b6](https://github.com/kythe/kythe/commit/407fa2b69f9a282b85241d4cfe350de686fdee72))
* **corpus checker:**  allow specifying an allowlist of corpora (#5033) ([14585948](https://github.com/kythe/kythe/commit/145859480b64849c1911cebfc09c42be69835476))
* **cxx_common:**  properly index generated protos (#4984) ([19c7da51](https://github.com/kythe/kythe/commit/19c7da51372f23809575331a6714ce51b8a29cb1))
* **cxx_indexer:**
  *  Allow VNames as NodeIds for #5037 (#5041) ([3040754f](https://github.com/kythe/kythe/commit/3040754fd0e341ec61cf5b23c9ac2ddd69280327))
  *  Read the semantic field into metadata for #5037 (#5040) ([06c49fc1](https://github.com/kythe/kythe/commit/06c49fc1d52acd42b47c26049ddaf3fa618e496b))
  *  Add semantics to metadata for #5037 (#5038) ([96ed0a4b](https://github.com/kythe/kythe/commit/96ed0a4ba0ccc0a05eab007f0766f4311cff3419))
  *  use default corpus for meta, builtin nodes (#5008) ([fd30e097](https://github.com/kythe/kythe/commit/fd30e09760112e1be760bbaee9fb3b741c959b48))
  *  add kRefFile to EdgeKinds (#4931) ([c0e92993](https://github.com/kythe/kythe/commit/c0e929931375e985e2dc0eacdcb6ca1614bac679))
  *  r/w refs and influence for += style operators (#4909) ([9dd66234](https://github.com/kythe/kythe/commit/9dd66234bb4d1c3b9f5e3759cd1dd5c23fb13435))
  *  support ++ and -- for influence and r/w refs (#4908) ([fe3b10dc](https://github.com/kythe/kythe/commit/fe3b10dce54dc301c3ac258450845d9c9ed18ee3))
* **dev:**  add rustfmt checks in linter (#4989) ([f21f37e4](https://github.com/kythe/kythe/commit/f21f37e4adaddf00fda085ac59927f84bc29bed6))
* **empty_corpus_checker:**  print out all corpora seen in the input (#5027) ([b263fd0d](https://github.com/kythe/kythe/commit/b263fd0dfc98fda6d9628b4bb5fa93ba82f85ffe))
* **extraction:**
  *  accept "arguments" in addition to "command" in compdb (#4986) ([c71a871e](https://github.com/kythe/kythe/commit/c71a871e3e66598fa1115036564f931e1bcdc8bd))
  *  support tree artifacts in bazel extractor (#4932) ([147034d1](https://github.com/kythe/kythe/commit/147034d195bff4cfe7b2072a9a3bbffa65276751))
* **go_indexer:**
  *  load inline metadata (#5001) ([23a9265f](https://github.com/kythe/kythe/commit/23a9265f6e09170793dcb0448adf3f5435fcb701))
  *  add option to put tapp nodes in the CU corpus (#4951) ([a367ca6f](https://github.com/kythe/kythe/commit/a367ca6ff4fd9d9ea99a5d7b3fec27a4e5cc9027))
* **indexing:**  add metadata support for generated protos (#5021) ([baed7a22](https://github.com/kythe/kythe/commit/baed7a223701f49b10c5823a507ebd57274b57c6))
* **java:**
  *  add empty corpus test and assign corpus to PACKAGE nodes (#5026) ([24740e0e](https://github.com/kythe/kythe/commit/24740e0ef86c421cfd18a9c689818654171e6914))
  *  emit visibility and other modifier facts (#4886) ([04347f60](https://github.com/kythe/kythe/commit/04347f60f392ff088799fbd48ab2cdb403f29de8))
* **java indexer:**  use compilation corpus for jvm and jdk nodes (#5035) ([fb7df986](https://github.com/kythe/kythe/commit/fb7df9864a30d94c5feeee9e1feca2d5f90ae5ea))
* **java_indexer:**
  *  place builtins in the compilation unit corpus (#4976) ([0291fced](https://github.com/kythe/kythe/commit/0291fcede6f6b50c113a8a97df6d0fdbfeedfb3c))
  *  add option to put tapps in the CU corpus (#4960) ([0198d461](https://github.com/kythe/kythe/commit/0198d461e84cf94d1d5e2393bf685c13122b37e0))
* **kzip info:**  check for invalid absolute paths (#5022) ([3bb00e01](https://github.com/kythe/kythe/commit/3bb00e01d705324042414bcbca5f997c9ca29d70))
* **post_processing:**  implement extends_overrides. (#4995) ([6889771f](https://github.com/kythe/kythe/commit/6889771f8cc764248b8cd5b6c523b25c59724eff))
* **proxy:**  Add support for transmitting protos in wire format (#4983) ([ae132ac1](https://github.com/kythe/kythe/commit/ae132ac1df57e7e80e34c4824d308884bff75015))
* **rust_common:**  prepare for Kythe release (#5055) ([a5f0fdfa](https://github.com/kythe/kythe/commit/a5f0fdfaee458ba5b2cf843ab1d37c984b8b34d2))
* **rust_extractor:**
  *  add support for processing vname configuration (#5053) ([3139303d](https://github.com/kythe/kythe/commit/3139303d761bd4a774b8f422550232503c7ac8cf))
  *  add script to automatically set LD_LIBRARY_PATH (#4998) ([8a4c2498](https://github.com/kythe/kythe/commit/8a4c2498d80132900a5f79c05415a8d7f128f82f))
* **rust_indexer:**
  *  add cli argument for temporary directory (#5051) ([6faca9f8](https://github.com/kythe/kythe/commit/6faca9f871e5bf17ff31d6998a7391a04f4519ac))
  *  Add support for communicating with the analyzer driver (#4985) ([7e542813](https://github.com/kythe/kythe/commit/7e54281377ff20dd6150bb7cf811f9a13f236732))
  *  Get source file contents from the FileProvider (#4974) ([70e517d4](https://github.com/kythe/kythe/commit/70e517d4f8c5df1bcf4a1d6f42d789f2a7414095))
  *  generate and check in rust protos (#4949) ([703eb64f](https://github.com/kythe/kythe/commit/703eb64f5b7ebd17363b62d5606aac5f71ba5a03))
* **schema:**  add ref/writes to edge kinds (#4933) ([ddb0c19c](https://github.com/kythe/kythe/commit/ddb0c19c3cfc074b42668b407c8a7a353c5e3df5))
* **site:**  add a target for serving website locally (#4970) ([b43e03f4](https://github.com/kythe/kythe/commit/b43e03f4945d4e774f04eb85dec7fa320df0a485))
* **testing:**  refactor empty corpus test and add one for golang (#5017) ([dd19c9ff](https://github.com/kythe/kythe/commit/dd19c9fffa4467055fd9dafebff63813a8ab9054))

#### Performance

* **serving:**  default to more performant approximate totals (#4896) ([0ebe1919](https://github.com/kythe/kythe/commit/0ebe19191b9468810df6416fbfaa9058a554d895))

## [v0.0.52] - 2021-04-09

#### Features

* **build:**  allow in-tree lex/yacc toolchains (#4880) ([ad8f0987](https://github.com/kythe/kythe/commit/ad8f0987e33d4d78c3ba3c8fc33ced9c8213cb07))
* **common:**  allow named captures in vnames.json rules (#4883) ([4398f3f5](https://github.com/kythe/kythe/commit/4398f3f57060d7bfeb320d0c885df7905592d756))
* **verifier:**  add stub for souffle support (#4881) ([fe68af99](https://github.com/kythe/kythe/commit/fe68af990c367228aeb4ad0f9a5403e3fa7defd3))

#### Bug Fixes

* **go_vnames:**  add go vname archives to simple_vnames.json (#4882) ([44e45a63](https://github.com/kythe/kythe/commit/44e45a6388e7780b11574906e80a18cc323d6f02))

## [v0.0.51] - 2021-04-05

#### Bug Fixes

* **go_vnames:**  properly map .x file vnames (#4878) ([00d8e038](https://github.com/kythe/kythe/commit/00d8e0387ad3e15a4295e8af72e8070a45c90fc8))

## [v0.0.50] - 2021-03-31

#### Bug Fixes

*   allow print_extra_actions to print ObjcCompile (#4838) ([2d995633](https://github.com/kythe/kythe/commit/2d995633c7f8424177c49aded94d9e2f904301f6))
*   revert #4813 (breaks darwin) (#4815) ([50286c1c](https://github.com/kythe/kythe/commit/50286c1cb8a3d0041ae50bfbe8e0e92d50fb83af))
* **bazel_go_extractor:**  include .x files in required input (#4874) ([a0a72617](https://github.com/kythe/kythe/commit/a0a72617af23375114546d4d75d3d74cf887a13b))
* **cxx_indexer:**  function definitions should influence declarations (#4812) ([c741a386](https://github.com/kythe/kythe/commit/c741a3865d01e43c684d5d177566d7b6140d74d2))
* **cxx_verifier:**  properly attribute srcs/hdrs to avoid ODR (#4821) ([b802f32b](https://github.com/kythe/kythe/commit/b802f32b25231dc28e37ed060c0a601486e827a6))
* **docs:**  Use rules_ruby to hermetically generated the web site  (#4857) ([f975ce26](https://github.com/kythe/kythe/commit/f975ce2607972c8a20e761f227d4f88a85ebbe65))
* **go_indexer:**  fallback to method ownership by pos (#4867) ([4097379d](https://github.com/kythe/kythe/commit/4097379d37e325220d0192df054c7869e8d8c2c9))
* **java:**
  *  emit "static" facts for classes and methods (#4861) ([9c061944](https://github.com/kythe/kythe/commit/9c06194450c2c78e5163545b5eee9d4f09dbf3fa))
  *  ensure a corpus is always set in extracted compilation units (#4849) ([e7326868](https://github.com/kythe/kythe/commit/e7326868072e30aa8af05943c6c0bb31949695d2))
* **platform:**  use proper JSON marshalling in proxy (#4804) ([d5a03e10](https://github.com/kythe/kythe/commit/d5a03e103f021645dc07ce716dbb3284f410f369))
* **proto:**  only decorate the relative type name in fields (#4872) ([d24403f2](https://github.com/kythe/kythe/commit/d24403f2ddfe68e1d1da19b23ce8b000c512352e))
* **serving:**
  *  move defer to after error check (#4862) ([6e4b9aea](https://github.com/kythe/kythe/commit/6e4b9aea748bccf28100f9b56b18f7835f2f0b63))
  *  use nearest FileInfo for revision data (#4827) ([f734fb09](https://github.com/kythe/kythe/commit/f734fb098b5cdb9db5b8a2406318b3bb2cfc8dc7))
* **tooling:**
  *  guard adding nil Entry to EntrySet (#4835) ([24735fa1](https://github.com/kythe/kythe/commit/24735fa1051a782ce870500a53549b73d8501ed2))
  *  use protojson for metadata marshalling (#4806) ([2d265641](https://github.com/kythe/kythe/commit/2d265641400536fd0f9fd2dbf8acc20046b2ed38))
  *  use protojson for Entry stream encoding (#4805) ([6eaf95d2](https://github.com/kythe/kythe/commit/6eaf95d278be9f8649c63e98e48822a71aab28b9))
* **typescript:**
  *  prepare for breakage for upcoming 4.2 migration (#4864) ([9abccb21](https://github.com/kythe/kythe/commit/9abccb214ce52f14dbc7571cfa96cc20399a10aa))
  *  tslints (#4843) ([891120a3](https://github.com/kythe/kythe/commit/891120a32edf1264224c767179e5051e55ce7249))
* **verifier:**
  *  use absl::Span instead of std::initializer_list elsewhere (#4834) ([875a36f6](https://github.com/kythe/kythe/commit/875a36f66bf8b83db20eb297dbcaabd9519d59f7))
  *  refactor the assertion AST into a library (#4814) ([de6b0c42](https://github.com/kythe/kythe/commit/de6b0c420020bbdbff32d743f73377d84a78f1c2))

#### Features

* **cxx_indexer:**
  *  add fields to the influence graph (#4820) ([77f6424a](https://github.com/kythe/kythe/commit/77f6424ac3d1e81dd677967014e679d49d3bbaf6))
  *  track influence through vardecls (#4818) ([b7025ca0](https://github.com/kythe/kythe/commit/b7025ca057559790ca8a77e84d547dbf31f32fe9))
* **docs:**  begin to document the influence relation (#4791) ([ce3200a2](https://github.com/kythe/kythe/commit/ce3200a25098e1f2667949fca563d8bd85b4acd3))
* **indexing:**  add synchronous FDS method (#4817) ([eaea3792](https://github.com/kythe/kythe/commit/eaea379277db915d6bc89502a081ce5250504ae2))
* **java_extractor:**  allow specifying java sources batch size for extractor (#4823) ([3bb67881](https://github.com/kythe/kythe/commit/3bb67881e2f29eae848822da7a64d77feaa47514))
* **schema:**  add ref/id to edge kinds (#4840) ([fbad5f9a](https://github.com/kythe/kythe/commit/fbad5f9aa330e54bdd4505b9871096a26c9517b7))
* **serving:**
  *  handle dirty_buffer in columnar decorations (#4858) ([adb5eb51](https://github.com/kythe/kythe/commit/adb5eb51b218b2531d4708f3e0ba58de5bbd49eb))
  *  handle dirty_buffer in columnar decorations (#4858) ([cbf4a453](https://github.com/kythe/kythe/commit/cbf4a45375c2830de6dc91fb57cc1df3d3d3de30))
* **typescript:**  returns influence functions (#4841) ([577446b7](https://github.com/kythe/kythe/commit/577446b7a0f35b86e4ab1782224eca2be8d204c6))
* **verifier:**
  *  don't spend time sorting input for the fast solver (#4863) ([7f4c3f03](https://github.com/kythe/kythe/commit/7f4c3f03dbd26427ba11db9fb5e96c9f1f60f035))
  *  convenience functions for building predicates and ranges (#4833) ([1b160dab](https://github.com/kythe/kythe/commit/1b160dab4c150489dcaae07990429296800cd614))
  *  set up a library/tests for lowering assertions to souffle (#4832) ([c8df46d2](https://github.com/kythe/kythe/commit/c8df46d2262b24e5afba54d27e758cd15ea22d7d))

## [v0.0.49] - 2021-01-22

#### Bug Fixes

*   disable TS function deprecation tagging (#4793) ([2c0bafa8](https://github.com/kythe/kythe/commit/2c0bafa81eaa104c79c07fcadad93b52121c58bc))
*   avoid non-inclusive language (#4683) ([18401be2](https://github.com/kythe/kythe/commit/18401be27d56d8d98a9e4dd61bb012944fa280b3))
* **build:**  use @io_kythe// instead of @// in bazel build file template (#4711) ([ca9e8881](https://github.com/kythe/kythe/commit/ca9e888151f0066c2aa49b0ddfd2d5600452fefa))
* **cxx_common:**
  *  change selector serialization to work better with protos (#4785) ([55231f7e](https://github.com/kythe/kythe/commit/55231f7e76f202827e4fe3fcf36270d36543414f))
  *  always initialize fields in FileOutputStream (#4757) ([22f821c5](https://github.com/kythe/kythe/commit/22f821c540d30d2bcc4cb7d25ba62d64f3394e3b))
* **cxx_indexer:**
  *  add missing cases and remove default from clang enum switches (#4799) ([a6804702](https://github.com/kythe/kythe/commit/a680470234442453571f33bf2140a7a361719edd))
* **java_common:**  keep rooted URI paths rooted (#4691) ([4f96ed5d](https://github.com/kythe/kythe/commit/4f96ed5deb79aeff912115dd5e943a1250a42afd))
* **java_indexer:**
  *  add MarkedSource for class initializer (#4758) ([58bf4fad](https://github.com/kythe/kythe/commit/58bf4fad805dca19ff94b9d4b25c8da4bc550203))
  *  emit references to .class literals (#4756) ([a54f6970](https://github.com/kythe/kythe/commit/a54f6970c0c1f3fd5e7d5d89b370c06dbb5e8394))
  *  handle doc refs to primitives (#4741) ([76939133](https://github.com/kythe/kythe/commit/769391337c1ea53d747b505123b82e89b60d3064))
  *  blame callsites under static fields on the clinit (#4740) ([625299dc](https://github.com/kythe/kythe/commit/625299dcfb62b32fc96e8850fa6d25a990e075dc))
  *  add zero-length definition for cinit (#4739) ([f43841b6](https://github.com/kythe/kythe/commit/f43841b64c685249c560d2567834fbb00f8ead61))
  *  special-case builtin Array class lookup (#4738) ([a4441336](https://github.com/kythe/kythe/commit/a44413369537121a9119ed3dbd101e9e2e1fb172))
  *  blame instance/static callsites on ctor/cinit methods (#4734) ([34ee6628](https://github.com/kythe/kythe/commit/34ee6628d28da2389731765a644829f74df4149d))
* **verifier:**  correctly pull out edge kinds and cache empty vnames/symbols (#4769) ([5d627e80](https://github.com/kythe/kythe/commit/5d627e80bb71c63ed460ae7b21795620e15a1aa4))

#### Features

*   create a localrun script (#4235) ([dd6d7c96](https://github.com/kythe/kythe/commit/dd6d7c96cee338315896fccb96814755a4eed98d))
* **api:**
  *  add revision to non-anchor locations that may ref a file (#4699) ([51c92b9b](https://github.com/kythe/kythe/commit/51c92b9b98eb486f457611169c2c056da5562c06))
  *  add revision fields for file/anchor locations (#4698) ([2918c49b](https://github.com/kythe/kythe/commit/2918c49bd417b344255cff4241ea45b628715e01))
* **common:**  add kytheuri CorpusPath helpers (#4707) ([371a5d65](https://github.com/kythe/kythe/commit/371a5d65c3563caf7803a2730feb53de9836a0c6))
* **cxx_common:**
  *  allow any range-of-string-like-types for RegexSet::Build (#4787) ([9e57829d](https://github.com/kythe/kythe/commit/9e57829d0d70226dd89401158ce7a06c4bbef684))
  *  allow matching with RE2 pattern in addtion to flat_hash_set (#4779) ([761465a6](https://github.com/kythe/kythe/commit/761465a6f9f040bcb0a9b73a77255c3b3af3b937))
  *  extend bazel artifact selection to allow stateful selectors (#4764) ([228c3070](https://github.com/kythe/kythe/commit/228c3070500969485d5f63c085a03477d7970633))
* **cxx_extractor:**
  *  use a stable working_directory if possible (#4771) ([410f69c5](https://github.com/kythe/kythe/commit/410f69c5bcb69fabcb78a5200b7631a1bffabd31))
  *  allow specifiying build target name in environment (#4686) ([a95f69ab](https://github.com/kythe/kythe/commit/a95f69ab37b9a9b261639fa7f1608fd9cc6238bd))
* **cxx_indexer:**
  *  add user-provided template instantiation excludelist (#4701) ([43f7d4de](https://github.com/kythe/kythe/commit/43f7d4de4a78ebb8538b05eef8ce4c4f7d49306d))
  *  opt-in to emitting influence edges (#4700) ([d62cf15c](https://github.com/kythe/kythe/commit/d62cf15c7e640aa9c3a6741a5e014f22e35e4a4a))
  *  trace influence through function calls (#4696) ([16c6730f](https://github.com/kythe/kythe/commit/16c6730fd5e86806961b6cd2ee6bf14473ce60b1))
  *  distinguish field writes from reads (#4687) ([582c4430](https://github.com/kythe/kythe/commit/582c4430bee03c7adfbece86cf32ecc8e5336a11))
  *  add blame scopes for field references (#4682) ([61a22a16](https://github.com/kythe/kythe/commit/61a22a16fee8c6652f4f0bbcd13246c23fab2100))
* **java_indexer:**  tag abstract methods/classes as such (#4746) ([0d455662](https://github.com/kythe/kythe/commit/0d455662e33ef14e3f63b6bfd286ae1ef0bef84d))
* **serving:**
  *  plumb revision data for XRefService responses (#4710) ([22f144ad](https://github.com/kythe/kythe/commit/22f144ade35e00e0a0b19305479cdd19e78b5d15))
  *  add revision serving fields for #4699/#4698 (#4708) ([d38aa957](https://github.com/kythe/kythe/commit/d38aa957b2ec96b2ca14f77d19b0c5fc220339c2))
* **textproto:**
  *  allow compilation units to contain multiple sources (#4772) ([37d71252](https://github.com/kythe/kythe/commit/37d7125261c6f57c9f6d24adc49e503d473c5074))
  *  expose DescriptorPool in plugin api (#4721) ([c9b8c15c](https://github.com/kythe/kythe/commit/c9b8c15c1b9e2087faa4e6eb4a2dd79658490d9d))
  *  put example plugin behind a flag (#4695) ([16a2eb53](https://github.com/kythe/kythe/commit/16a2eb53a49d459ff99b730dbafc8f3b3a71b614))
  *  accept proto_library targets instead of .proto files (#4689) ([659de4d2](https://github.com/kythe/kythe/commit/659de4d29670922574ff9a4371d118d5b565cdf4))
* **tooling:**  add optional analysis timeout to driver (#4714) ([14d4fa59](https://github.com/kythe/kythe/commit/14d4fa594301e53159cb99e6f048d90a65d2e434))
* **verifier:**
  *  build a souffle frontend that can read an entrystream (#4768) ([0e9dc6a9](https://github.com/kythe/kythe/commit/0e9dc6a9e37c907959736dbc2fb436feaf684b8c))

## [v0.0.48] - 2020-09-01
* **runextractor:**
  * Pass flags to the compdb extractor (#4663) ([38fd346](https://github.com/kythe/kythe/commit/38fd346e0bbbf101334d7101a4031c320ac5c269))

## [v0.0.47] - 2020-08-18
* Include stacktraces from assertion failures in C++ tools (#4655) ([784e8ee7](https://github.com/kythe/kythe/commit/784e8ee7da2c5fe8eb8c246393f9d700261fd344))

## [v0.0.46] - 2020-08-13

#### Features

*   Create new Rust Bazel extractor (#4602) ([a030ce48](https://github.com/kythe/kythe/commit/a030ce48ddf46b171736d136af10f8462c5a302a))
*   Add library for generating Rust save_analysis files (#4594) ([c84b2373](https://github.com/kythe/kythe/commit/c84b2373f36aa783a8d2ef9db5050dab78655b61))
*   Add support for defining environment variables passed to extractors (#4592) ([ade706d5](https://github.com/kythe/kythe/commit/ade706d5409a9a997f85d14a1251033577f1f5a1))
* **cxx_indexer:**
  *  record var-to-var influence (#4559) ([e8461650](https://github.com/kythe/kythe/commit/e8461650c5665aa6ead02a6b35124b10ab5598a1))
  *  annotate edges we know are writes to vars and ivars (#4558) ([c3f64566](https://github.com/kythe/kythe/commit/c3f645667b9bb72747f1ac9f0d63c84099537a03))
  *  blame variable references on the surrounding context (#4556) ([b9ea1a9c](https://github.com/kythe/kythe/commit/b9ea1a9c691fd994e6ae9dcd8ebfb1fb157b7173))
* **extraction:**  generate build metadata kzip in bazel-extractor docker image (#4554) ([7a528667](https://github.com/kythe/kythe/commit/7a5286676e1d86fd3fa1987b8039c2615c8a3f2f))
* **fuchsia:**  Fuchsia-specific extractor (#4624) ([38346f2e](https://github.com/kythe/kythe/commit/38346f2eb38ba0e73c72cf24884e4dbbedb07447), closes [#4606](https://github.com/kythe/kythe/issues/4606))
* **go-extractor image:**  record commit timestamp in kzip (#4582) ([5813326c](https://github.com/kythe/kythe/commit/5813326c593ed7be77ceee2f9cb3c3cfd63501c7))
* **java_common:**  add ByteString overloads for fact value conversions (#4610) ([a95d2e06](https://github.com/kythe/kythe/commit/a95d2e0620202fa97169e6091cad03eb191d0183))
* **rust:**
  *  Add support for Rust cross-references (#4641) ([8555988e](https://github.com/kythe/kythe/commit/8555988e1ed6305c6ebbe3c32c9ab41a3410a519))
  *  Makes the extractor more robust (#4640) ([358eac11](https://github.com/kythe/kythe/commit/358eac11735b802d2686f0f20e301c179383f1b2), breaks [#](https://github.com/kythe/kythe/issues/))
  *  Support emitting remaining definition types (#4638) ([61dc64c1](https://github.com/kythe/kythe/commit/61dc64c104ff05dde731991e9e4ae897a1531428))
  *  Add support for indexing enums (#4633) ([b9844877](https://github.com/kythe/kythe/commit/b9844877a2f826cca2f1d7e3c156136b16cc88d1))
  *  Adds Fuchsia Rust extractor to release (#4634) ([29d7f12e](https://github.com/kythe/kythe/commit/29d7f12eabffb6f1d26f4fa2d5a07c0284a9f880))
  *  Add support to the Rust indexer for emitting module definitions and anchors (#4629) ([405ee13f](https://github.com/kythe/kythe/commit/405ee13f587b0323bf764328679fbb69137cc7fa))
  *  Support emitting function definitions (#4617) ([b2876116](https://github.com/kythe/kythe/commit/b2876116a9f2795569e51f0e128ce603ce94c997))
  *  Add Bazel rule for running Rust indexer tests (#4612) ([a0f1f67b](https://github.com/kythe/kythe/commit/a0f1f67bdbd61698bbfc78dc16b8ae475478e64a))
  *  Create Rust indexer CLI (#4605) ([2dba4dd9](https://github.com/kythe/kythe/commit/2dba4dd904cbbfff5a2ff3dcde5f7b5b930d23d5))
  *  Unify Kythe Rust dependencies (#4611) ([545968b9](https://github.com/kythe/kythe/commit/545968b919f76a41086eb7db089597dabcf13917))
  *  Create KytheWriter component for Rust indexer (#4565) ([bf8c25a8](https://github.com/kythe/kythe/commit/bf8c25a8446dbc3a0b60dc00a0695ac659ae22a8))
  *  Change Rust indexer FileProvider (#4564) ([7852fc43](https://github.com/kythe/kythe/commit/7852fc43ebd660a461f06294444a4362e8bd4b99))
  *  Create provider and error modules for Rust indexer library (#4550) ([d47857e7](https://github.com/kythe/kythe/commit/d47857e7de04af0f33b60224f649c7daec4bf43f))
* **textproto:**  index enum values (#4615) ([573ffff4](https://github.com/kythe/kythe/commit/573ffff49ea75ca6c859da72046e1a6fe1d8d63a))

#### Breaking Changes

* **rust:**  Makes the extractor more robust (#4640) ([358eac11](https://github.com/kythe/kythe/commit/358eac11735b802d2686f0f20e301c179383f1b2), breaks [#](https://github.com/kythe/kythe/issues/))

#### Bug Fixes

*   Remove old cargo-kythe directory (#4601) ([e71ea980](https://github.com/kythe/kythe/commit/e71ea98001182e0c1216330aca13bc77c58631d8))
*   Clean up old Rust tools (#4600) ([9e316b15](https://github.com/kythe/kythe/commit/9e316b15da8fb214869dde0d18d9350fb340ab0b))
*   allow PushScope to work with braced-init (#4567) ([70374dff](https://github.com/kythe/kythe/commit/70374dffb94eea48ff781a7177fdf296f5749314))
* **bazel extractor:**  fix path to bazelisk in docker image (#4555) ([468ad47f](https://github.com/kythe/kythe/commit/468ad47f48c02d5ce20eeb64166f489b25e13f4c))
* **cxx_indexer:**
  *  avoid an assert check in Clang, silence errors (#4631) ([0a700ac0](https://github.com/kythe/kythe/commit/0a700ac06fb17b7c64213692d545e8e76aa18414))
  *  handle null init expr (#4622) ([bcdc8e9d](https://github.com/kythe/kythe/commit/bcdc8e9df40a540ca4e04c75f95f83b0950c264b))
  *  report errors more directly rather than stderr (#4613) ([9e12b4a4](https://github.com/kythe/kythe/commit/9e12b4a44e7361c7c2e6a622cd32f2ad65e3a0c1))
  *  handle list-init on incomplete types (#4584) ([0bcb51ef](https://github.com/kythe/kythe/commit/0bcb51efd42a02fefe0a1442dc938c91f5725b3f))
  *  increase flexibility of proto library plugin (#4580) ([d5f3e41b](https://github.com/kythe/kythe/commit/d5f3e41baae5e886c1388d6ee9608f8b5c1d7581))
  *  make sure init-list-expr has a type (#4571) ([bfc03501](https://github.com/kythe/kythe/commit/bfc0350165abac1e9825c11f5bce4807135caddc))
  *  properly filter empty ref/init exprs (#4560) ([9fd021cd](https://github.com/kythe/kythe/commit/9fd021cd1aacdc1a473d1826f41d7f43a020d1d8))
* **java_indexer:**  workaround JDK bug on Java 11 (#4614) ([15cea9b3](https://github.com/kythe/kythe/commit/15cea9b3825b65d416004946778181598f54cd6a))
* **rust:**
  *  Fixes the kzip generation (#4650) ([5b74224c](https://github.com/kythe/kythe/commit/5b74224c2cdc908b584818eddc0f24178eba1516))
  *  Fix definition anchors and xrefs in the indexer (#4645) ([00bb6402](https://github.com/kythe/kythe/commit/00bb6402244a451c6af11cc49989a67a9c2f8dd4))
  *  write_all instead of write in the extractor (#4646) ([8cab33ed](https://github.com/kythe/kythe/commit/8cab33ed2d69ed9d56820b082e4059b2fd0259ae))
  *  Fix Rust indexer issue with Trait definitions (#4644) ([8750956d](https://github.com/kythe/kythe/commit/8750956d168de91d2a33362200f011a533cf06f7))
  *  Fix bugs in the Rust implementation (#4642) ([bfc0cc58](https://github.com/kythe/kythe/commit/bfc0cc584b626725d6db4105bc3806653f05bd7a))
  *  Fix fuchsia_extractor test data (#4637) ([9e1eedcd](https://github.com/kythe/kythe/commit/9e1eedcd3cf347aa91a323957995369266afaf5f))
  *  Ensure that Rust extractor creates a top-level folder (#4616) ([297d4440](https://github.com/kythe/kythe/commit/297d44405b7b33e9b5e1810c8e504e9791e148fd))
* **schema:**
  *  add defines/implicit and examples (#4628) ([0a500b8f](https://github.com/kythe/kythe/commit/0a500b8f0a55f177919436cb5311a3bcb41e2d88))
  *  document the use of special spans for implicit modules (#4625) ([f1f9a814](https://github.com/kythe/kythe/commit/f1f9a814c3b3c07f5343623569d56592a0cb290e))
* **serving:**  turn diffmatchpatch panics into internal errors (#4621) ([88c36a09](https://github.com/kythe/kythe/commit/88c36a096254823ae72895439511d1b365fb88e1))
* **tools:**  kzip merge uses proto as default encoding (#4649) ([5b77cd64](https://github.com/kythe/kythe/commit/5b77cd640ecc1e642f9f4783fc5e2f925b006996))
* **verifier:**  compile with the xcode clang (#4553) ([9e75b8af](https://github.com/kythe/kythe/commit/9e75b8af12683655374d0d656ebfc449ad08b523))

## [v0.0.45] - 2020-06-11

#### Bug Fixes

* **cxx_indexer:**  don't emit ref/init for unspecified fields (#4516) ([6e60a52c](https://github.com/kythe/kythe/commit/6e60a52c7752085ff6a023913469e2fd40597016))
* **java_indexer:**
  *  don't emit JVM nodes for erroneous types (#4509) ([7f9e3d98](https://github.com/kythe/kythe/commit/7f9e3d98c60e01a9c2d4495ebf6743831365862d))
* **serving:**  avoid returning a nil node in map (#4487) ([4e303666](https://github.com/kythe/kythe/commit/4e3036668a59804cbc34c7e0a1bd9d987b81a294))
* **ts_indexer:**  emit ref/call edges from calls. (#4478) ([920aeaae](https://github.com/kythe/kythe/commit/920aeaae6631aa22be14a8bdc4965dfbdb44240c))

#### Features

*   Change kzip writer implementations to use proto kzip encoding by default. (#4547) ([566a83bd](https://github.com/kythe/kythe/commit/566a83bd8f84bb8474e9d4d4078e13fb4edb7567))
* **compdb:**
  *  print out command for failed extractions (#4546) ([da636ca2](https://github.com/kythe/kythe/commit/da636ca268677df5a3a6d7b372099a110fa42a0b))
  *  continue processing other compilations on error (#4544) ([a6022c91](https://github.com/kythe/kythe/commit/a6022c916611835a415c4e79a407806afabbdf20))
* **java_indexer:**  experimentally emit named edges to JVM nodes (#4490) ([9753c0f6](https://github.com/kythe/kythe/commit/9753c0f68392aad3f63d5133c317ad3b2c82227d))
* **kzip:**  add subcommand and proto for metadata kzips (#4545) ([f063dae5](https://github.com/kythe/kythe/commit/f063dae5813ed2e135ac57a93699dc493652709a))
* **rust:**
  *  Remove deprecated Rust indexer ([4140adfc](https://github.com/kythe/kythe/commit/4140adfca08c894828d0eb91c6c8d3b74fa1886e))
  *  Add basic Rust extractor ([b31406b8](https://github.com/kythe/kythe/commit/b31406b8523852b8e4f67dd64418c1a700b45f07))

## [v0.0.44] - 2020-04-21

#### Features

*   add lib for reading artifacts from bazel event streams (#4460) ([0a124804](https://github.com/kythe/kythe/commit/0a12480436fdd76d3cca5e2585f94cb73bd1dcee))
* **api:**  return Documentation node defs even if no facts are found (#4458) ([f03e8e67](https://github.com/kythe/kythe/commit/f03e8e673ef556e8e7b37935f38cfba61d858b86))
* **build:**  have arc run build test with named config (#4438) ([756521e8](https://github.com/kythe/kythe/commit/756521e8aec17cd5a607be841f95617f6d1f899d))
* **cxx_extractor:**  cache symlink resolution in PathRealizer (#4483) ([0f3c7f47](https://github.com/kythe/kythe/commit/0f3c7f47491d30488bf85108e6a3cd539fc72d77))
* **cxx_indexer:**
  *  emit ref/id to class when class and ctor overlap (#4476) ([0f9ced3e](https://github.com/kythe/kythe/commit/0f9ced3e2490c0dcd2661d8da8cf27168885748d))
  *  expose a useful internal metadata loading function (#4459) ([f196b3e8](https://github.com/kythe/kythe/commit/f196b3e8b1321395f17ec3e3a5eb6e43e9094046))
* **indexing:**  add ref/id edge (#4435) ([82ce8fc6](https://github.com/kythe/kythe/commit/82ce8fc68e7669790b5c838b270696071978ef71))
* **runextractor cmake:**  add -extra_cmake_args option (#4436) ([489b5aec](https://github.com/kythe/kythe/commit/489b5aec0b1920ab528d6c9cd65f58dd690cbe97))

#### Bug Fixes

*   require flex version 2.6 or newer (#4456) ([70f2eeea](https://github.com/kythe/kythe/commit/70f2eeea4c642adbd1897efb91b1a4aab10bbec0))
* **cxx_indexer:**
  *  don't check-fail on copy-init-list (#4471) ([addc2410](https://github.com/kythe/kythe/commit/addc2410c5ae9dc94410361ba0185dd08c217f26))
  *  properly emit ref/init edges in all cases (#4468) ([62d89fe8](https://github.com/kythe/kythe/commit/62d89fe8c1bc7340341ef98874cf1b18ab1460b1))
  *  emit ref and ref/init for designated inits (#4462) ([5522b38a](https://github.com/kythe/kythe/commit/5522b38a555f7216eaa2502b2ecb91c53e8f98bc))
  *  emit zero-width spans for entities in macro bodies (#4461) ([118afbe2](https://github.com/kythe/kythe/commit/118afbe2d2c2e32f499eb2be4689b62bbb805002))
  *  update proto literal support to mirror current lib (#4439) ([74dfc7b2](https://github.com/kythe/kythe/commit/74dfc7b29c73d7111255fec3adccf6d600531a1b))
* **extraction:**  deterministically write files to kzip (#4479) ([04e2fc9e](https://github.com/kythe/kythe/commit/04e2fc9e0d959ebd3bbed6c1d2f3ff7675e5242c))
* **java_indexer:**
  *  only consider Symbols without a source file external (#4472) ([059806cf](https://github.com/kythe/kythe/commit/059806cfc546fe1a8507ed9107da401329661462))
  *  ensure annotated type vars generate consistent VNames (#4441) ([fd91ce0a](https://github.com/kythe/kythe/commit/fd91ce0a0a9b8e6878c6ba576985a2e59247e9d1))
  *  only apply jdk override for non-source symbols (#4434) ([1d4ced6c](https://github.com/kythe/kythe/commit/1d4ced6c7da3c33cf367b993d59da7424e19ac55))
* **java_tests:**  export jdk_kzip (#4443) ([c76ad892](https://github.com/kythe/kythe/commit/c76ad89265154351b27725d49d5cd97795d0b046))
* **ts_indexer:**  bug related to overridden functions. (#4463) ([7288552a](https://github.com/kythe/kythe/commit/7288552abf729989c10d75e7256fe482cd665a12))

## [v0.0.43] - 2020-03-10

#### Bug Fixes

* **bazel_go_extractor:**  record canonical importpath for archives (#4425) ([9f999295](https://github.com/kythe/kythe/commit/9f99929540d712f534425e15d1b9016d3589f4f1))
* **cmake docs:**  make output directory before extracting (#4409) ([2ceb0305](https://github.com/kythe/kythe/commit/2ceb03051225c83b43aae688440e4bcaab12a42c))
* **cxx_indexer:**
  *  emit ref to class from ctor (#4400) ([3a9b2a7c](https://github.com/kythe/kythe/commit/3a9b2a7c8499eaaf2001792a31b3b43ad58f10dc))
* **objc_tests:**  run objc tests on more platforms (#4426) ([9ddf0176](https://github.com/kythe/kythe/commit/9ddf017685d189ede377d9a4ae9dc925e1a4f75e))
* **runextractor:**  set --build_file default to build.gradle (#4393) ([eff63cb2](https://github.com/kythe/kythe/commit/eff63cb2f333f030bee12beada233c5c146100b1))

#### Features

* **build:**  switch to an autoconfigured ubuntu 18.04 image and C++17 (#4385) ([509d7c61](https://github.com/kythe/kythe/commit/509d7c618a5203682566fbcf5fc0572a87dddef2))
* **example:**  recommend a way to handle TypeScript/&c-style modules (#4357) ([9e9a6571](https://github.com/kythe/kythe/commit/9e9a6571d2d557b4fd84c3a3e0c6a2c98d28d985))
* **go_indexer:**  support GeneratedCodeInfo .meta textproto files (#4414) ([7c44d34c](https://github.com/kythe/kythe/commit/7c44d34c8a4b92d2821dbf9abaf672fa07c20791))
* **java_extractor:**  attribute corpus based on sources if unambiguous (#4399) ([7f3868cf](https://github.com/kythe/kythe/commit/7f3868cf9c6fc6dc34646a4ef15affdbdd054bef))

## [v0.0.42] - 2020-02-19

#### Bug Fixes

* **copy_kzip:**
  *  hard links always fail under bazel so don't try (#4365) ([10540135](https://github.com/kythe/kythe/commit/10540135260c2559171ad4ad68844784133773e9))
* **cxx_indexer:**
  *  address marked-source differences on builtin functions (#4379) ([cbd9244c](https://github.com/kythe/kythe/commit/cbd9244c52c9deb0f40772f8e0b5baaf9846be5f))
  *  emit exactly one file-file generates edge for proto (#4377) ([5ab67169](https://github.com/kythe/kythe/commit/5ab6716903a631d8543f11b39c8ebed6fc8d2fe6))
  *  use anchor not decl for file-file generates endpoint (#4372) ([6e14dbde](https://github.com/kythe/kythe/commit/6e14dbde996a9edb2cb07c3ede85298be856149e))
* **go_indexer:**  use the anchor for the file-file generates edges (#4348) ([3d658d1d](https://github.com/kythe/kythe/commit/3d658d1d45deeac5f77d164d9e1b45d7d76f5872))
* **tooling:**
  *  ensure VNameRewriteRule JSON encoding is canonical (#4352) ([124e000a](https://github.com/kythe/kythe/commit/124e000a1f4998813805f698483334ca160b571a))
  *  keep track of added source file paths (#4351) ([e753a99a](https://github.com/kythe/kythe/commit/e753a99a87ee85c29a86aff61ba983917104fe0d))

#### Features

* **cxx_indexer:**
  *  add test for proto/c++ xrefs with included proto (#4363) ([90f7c4e0](https://github.com/kythe/kythe/commit/90f7c4e0a696d91de8a00d2e33d85806a268996f))
  *  emit USRs for macros (#4315) (#4358) ([87d3be72](https://github.com/kythe/kythe/commit/87d3be72bf54b1f65bde11ba02df79de2dd05304))
* **extractors:**  add a passthrough bazel extractor (#4354) ([e22e87aa](https://github.com/kythe/kythe/commit/e22e87aa1ae37b97a09f2d4317b0167d279f6f2f))
* **go extractor:**  relative paths against KYTHE_ROOT_DIRECTORY (#4380) ([18c0563f](https://github.com/kythe/kythe/commit/18c0563fcd9ffbca8f6b071e6a29078a4e8573e9))
* **go indexer:**  add option to only emit doc/uri facts for stdlib pkgs (#4383) ([255331cb](https://github.com/kythe/kythe/commit/255331cb1e861f1f9741c7afce56988c04a28658))
* **go_indexer:**  add generates edges for proto-generated files (#4337) ([2f01e628](https://github.com/kythe/kythe/commit/2f01e6281e6bf3ddc1cf15efb87dff3363cb1c3e))
* **kzip_merge:**  allow applying vname rules during merge (#4366) ([21d68ce6](https://github.com/kythe/kythe/commit/21d68ce62bcd4396e494792287ad4591abe293b6))
* **tooling:**  add vnames utility for handling rewrite rules (#4347) ([4a75aef1](https://github.com/kythe/kythe/commit/4a75aef1848c689577eb31222a4f8495f667c17f))

## [v0.0.41] - 2020-01-31

#### Features

* **cxx_indexer:**  add generates edges for proto-generated files (#4332) ([299950d0](https://github.com/kythe/kythe/commit/299950d06d31c8d323e7b4de75050ca0585dde23))
* **java_extractor:**
  * allow passing search paths as a map (#4323) ([2ebd4c9f](https://github.com/kythe/kythe/commit/2ebd4c9f80daa3a0cce1ae156fb682e60baf3a56))
  * allow excluding modules from openjdk11 extraction (#4341) ([b09b9a7f](https://github.com/kythe/kythe/commit/b09b9a7fdf212be5f6158d2be61f03ef2000a2a2))
  * allow openjdk11 build and src dirs to differ (#4338) ([b6bb46d4](https://github.com/kythe/kythe/commit/b6bb46d4935b790abc124319a0aeb6219c91198d))
* **java_indexer:**  add generates edges for proto-generated java files (#4321) ([8f2080d4](https://github.com/kythe/kythe/commit/8f2080d48322e8b749c0753b769d9c5332fae934))

#### Bug Fixes

* **cxx_common:**  rework VNameRef to conform to VName, templatize VNameLess (#4331) ([2a83959d](https://github.com/kythe/kythe/commit/2a83959dc1f7851029347cb31a4b66c3c93a42b9))
* **go extractor:**  treat -arc flag like -importpath (#4324) ([0f75ac02](https://github.com/kythe/kythe/commit/0f75ac02d55cdc6e9b4fb5b1ab1a9d8927807f57))
* **java_extractor:**
  *  indirect runfiles path in opendjk11 extractor (#4335) ([0354d63a](https://github.com/kythe/kythe/commit/0354d63add4ed6dbdaa00ef70acd4af06aad7695))
  *  wrong number of logger arguments (#4330) ([e645e980](https://github.com/kythe/kythe/commit/e645e980fd0ac008782bc97f95f59a16f35e97a7))
  *  use a stable working directory if possible (#4329) ([75dd01f0](https://github.com/kythe/kythe/commit/75dd01f06ce9d1b15712d3d5a90d12d48e26075c))

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
* **runextractor:**  Tell cmake to use clang (#4319) ([5d10af8b](https://github.com/kythe/kythe/commit/5d10af8bb9bb129efb9cd632811399cfacf7095f))
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
  *  allow and test std::make_shared (#4059) ([4016c5b3](https://github.com/kythe/kythe/commit/4016c5b30a02a85e996c10e4a873e10d1b80c4cd))
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

[Unreleased] https://github.com/kythe/kythe/compare/v0.0.63...HEAD
[v0.0.63] https://github.com/kythe/kythe/compare/v0.0.62...v0.0.63
[v0.0.62] https://github.com/kythe/kythe/compare/v0.0.61...v0.0.62
[v0.0.61] https://github.com/kythe/kythe/compare/v0.0.60...v0.0.61
[v0.0.60] https://github.com/kythe/kythe/compare/v0.0.59...v0.0.60
[v0.0.59] https://github.com/kythe/kythe/compare/v0.0.58...v0.0.59
[v0.0.58] https://github.com/kythe/kythe/compare/v0.0.57...v0.0.58
[v0.0.57] https://github.com/kythe/kythe/compare/v0.0.56...v0.0.57
[v0.0.56] https://github.com/kythe/kythe/compare/v0.0.55...v0.0.56
[v0.0.55] https://github.com/kythe/kythe/compare/v0.0.54...v0.0.55
[v0.0.54] https://github.com/kythe/kythe/compare/v0.0.53...v0.0.54
[v0.0.53] https://github.com/kythe/kythe/compare/v0.0.52...v0.0.53
[v0.0.52] https://github.com/kythe/kythe/compare/v0.0.51...v0.0.52
[v0.0.51] https://github.com/kythe/kythe/compare/v0.0.50...v0.0.51
[v0.0.50] https://github.com/kythe/kythe/compare/v0.0.49...v0.0.50
[v0.0.49] https://github.com/kythe/kythe/compare/v0.0.48...v0.0.49
[v0.0.48] https://github.com/kythe/kythe/compare/v0.0.47...v0.0.48
[v0.0.47] https://github.com/kythe/kythe/compare/v0.0.46...v0.0.47
[v0.0.46] https://github.com/kythe/kythe/compare/v0.0.45...v0.0.46
[v0.0.45] https://github.com/kythe/kythe/compare/v0.0.44...v0.0.45
[v0.0.44] https://github.com/kythe/kythe/compare/v0.0.43...v0.0.44
[v0.0.43] https://github.com/kythe/kythe/compare/v0.0.42...v0.0.43
[v0.0.42] https://github.com/kythe/kythe/compare/v0.0.41...v0.0.42
[v0.0.41] https://github.com/kythe/kythe/compare/v0.0.40...v0.0.41
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
