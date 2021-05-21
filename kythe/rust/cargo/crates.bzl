"""
@generated
cargo-raze generated Bazel file.

DO NOT EDIT! Replaced on runs of cargo-raze
"""

load("@bazel_tools//tools/build_defs/repo:git.bzl", "new_git_repository")  # buildifier: disable=load
load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")  # buildifier: disable=load
load("@bazel_tools//tools/build_defs/repo:utils.bzl", "maybe")  # buildifier: disable=load

def raze_fetch_remote_crates():
    """This function defines a collection of repos and should be called in a WORKSPACE file"""
    maybe(
        http_archive,
        name = "raze__adler__1_0_2",
        url = "https://crates.io/api/v1/crates/adler/1.0.2/download",
        type = "tar.gz",
        sha256 = "f26201604c87b1e01bd3d98f8d5d9a8fcbb815e8cedb41ffccbeb4bf593a35fe",
        strip_prefix = "adler-1.0.2",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.adler-1.0.2.bazel"),
    )

    maybe(
        http_archive,
        name = "raze__aho_corasick__0_7_18",
        url = "https://crates.io/api/v1/crates/aho-corasick/0.7.18/download",
        type = "tar.gz",
        sha256 = "1e37cfd5e7657ada45f742d6e99ca5788580b5c529dc78faf11ece6dc702656f",
        strip_prefix = "aho-corasick-0.7.18",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.aho-corasick-0.7.18.bazel"),
    )

    maybe(
        http_archive,
        name = "raze__ansi_term__0_11_0",
        url = "https://crates.io/api/v1/crates/ansi_term/0.11.0/download",
        type = "tar.gz",
        sha256 = "ee49baf6cb617b853aa8d93bf420db2383fab46d314482ca2803b40d5fde979b",
        strip_prefix = "ansi_term-0.11.0",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.ansi_term-0.11.0.bazel"),
    )

    maybe(
        http_archive,
        name = "raze__anyhow__1_0_40",
        url = "https://crates.io/api/v1/crates/anyhow/1.0.40/download",
        type = "tar.gz",
        sha256 = "28b2cd92db5cbd74e8e5028f7e27dd7aa3090e89e4f2a197cc7c8dfb69c7063b",
        strip_prefix = "anyhow-1.0.40",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.anyhow-1.0.40.bazel"),
    )

    maybe(
        http_archive,
        name = "raze__assert_cmd__1_0_4",
        url = "https://crates.io/api/v1/crates/assert_cmd/1.0.4/download",
        type = "tar.gz",
        sha256 = "8f57fec1ac7e4de72dcc69811795f1a7172ed06012f80a5d1ee651b62484f588",
        strip_prefix = "assert_cmd-1.0.4",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.assert_cmd-1.0.4.bazel"),
    )

    maybe(
        http_archive,
        name = "raze__atty__0_2_14",
        url = "https://crates.io/api/v1/crates/atty/0.2.14/download",
        type = "tar.gz",
        sha256 = "d9b39be18770d11421cdb1b9947a45dd3f37e93092cbf377614828a319d5fee8",
        strip_prefix = "atty-0.2.14",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.atty-0.2.14.bazel"),
    )

    maybe(
        http_archive,
        name = "raze__autocfg__1_0_1",
        url = "https://crates.io/api/v1/crates/autocfg/1.0.1/download",
        type = "tar.gz",
        sha256 = "cdb031dd78e28731d87d56cc8ffef4a8f36ca26c38fe2de700543e627f8a464a",
        strip_prefix = "autocfg-1.0.1",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.autocfg-1.0.1.bazel"),
    )

    maybe(
        http_archive,
        name = "raze__bitflags__1_2_1",
        url = "https://crates.io/api/v1/crates/bitflags/1.2.1/download",
        type = "tar.gz",
        sha256 = "cf1de2fe8c75bc145a2f577add951f8134889b4795d47466a54a5c846d691693",
        strip_prefix = "bitflags-1.2.1",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.bitflags-1.2.1.bazel"),
    )

    maybe(
        http_archive,
        name = "raze__bstr__0_2_16",
        url = "https://crates.io/api/v1/crates/bstr/0.2.16/download",
        type = "tar.gz",
        sha256 = "90682c8d613ad3373e66de8c6411e0ae2ab2571e879d2efbf73558cc66f21279",
        strip_prefix = "bstr-0.2.16",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.bstr-0.2.16.bazel"),
    )

    maybe(
        http_archive,
        name = "raze__byteorder__1_4_3",
        url = "https://crates.io/api/v1/crates/byteorder/1.4.3/download",
        type = "tar.gz",
        sha256 = "14c189c53d098945499cdfa7ecc63567cf3886b3332b312a5b4585d8d3a6a610",
        strip_prefix = "byteorder-1.4.3",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.byteorder-1.4.3.bazel"),
    )

    maybe(
        http_archive,
        name = "raze__bzip2__0_3_3",
        url = "https://crates.io/api/v1/crates/bzip2/0.3.3/download",
        type = "tar.gz",
        sha256 = "42b7c3cbf0fa9c1b82308d57191728ca0256cb821220f4e2fd410a72ade26e3b",
        strip_prefix = "bzip2-0.3.3",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.bzip2-0.3.3.bazel"),
    )

    maybe(
        http_archive,
        name = "raze__bzip2_sys__0_1_10_1_0_8",
        url = "https://crates.io/api/v1/crates/bzip2-sys/0.1.10+1.0.8/download",
        type = "tar.gz",
        sha256 = "17fa3d1ac1ca21c5c4e36a97f3c3eb25084576f6fc47bf0139c1123434216c6c",
        strip_prefix = "bzip2-sys-0.1.10+1.0.8",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.bzip2-sys-0.1.10+1.0.8.bazel"),
    )

    maybe(
        http_archive,
        name = "raze__cc__1_0_67",
        url = "https://crates.io/api/v1/crates/cc/1.0.67/download",
        type = "tar.gz",
        sha256 = "e3c69b077ad434294d3ce9f1f6143a2a4b89a8a2d54ef813d85003a4fd1137fd",
        strip_prefix = "cc-1.0.67",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.cc-1.0.67.bazel"),
    )

    maybe(
        http_archive,
        name = "raze__cfg_if__0_1_10",
        url = "https://crates.io/api/v1/crates/cfg-if/0.1.10/download",
        type = "tar.gz",
        sha256 = "4785bdd1c96b2a846b2bd7cc02e86b6b3dbf14e7e53446c4f54c92a361040822",
        strip_prefix = "cfg-if-0.1.10",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.cfg-if-0.1.10.bazel"),
    )

    maybe(
        http_archive,
        name = "raze__cfg_if__1_0_0",
        url = "https://crates.io/api/v1/crates/cfg-if/1.0.0/download",
        type = "tar.gz",
        sha256 = "baf1de4339761588bc0619e3cbc0120ee582ebb74b53b4efbf79117bd2da40fd",
        strip_prefix = "cfg-if-1.0.0",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.cfg-if-1.0.0.bazel"),
    )

    maybe(
        http_archive,
        name = "raze__clap__2_33_3",
        url = "https://crates.io/api/v1/crates/clap/2.33.3/download",
        type = "tar.gz",
        sha256 = "37e58ac78573c40708d45522f0d80fa2f01cc4f9b4e2bf749807255454312002",
        strip_prefix = "clap-2.33.3",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.clap-2.33.3.bazel"),
    )

    maybe(
        http_archive,
        name = "raze__cloudabi__0_0_3",
        url = "https://crates.io/api/v1/crates/cloudabi/0.0.3/download",
        type = "tar.gz",
        sha256 = "ddfc5b9aa5d4507acaf872de71051dfd0e309860e88966e1051e462a077aac4f",
        strip_prefix = "cloudabi-0.0.3",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.cloudabi-0.0.3.bazel"),
    )

    maybe(
        http_archive,
        name = "raze__colored__2_0_0",
        url = "https://crates.io/api/v1/crates/colored/2.0.0/download",
        type = "tar.gz",
        sha256 = "b3616f750b84d8f0de8a58bda93e08e2a81ad3f523089b05f1dffecab48c6cbd",
        strip_prefix = "colored-2.0.0",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.colored-2.0.0.bazel"),
    )

    maybe(
        http_archive,
        name = "raze__crc32fast__1_2_1",
        url = "https://crates.io/api/v1/crates/crc32fast/1.2.1/download",
        type = "tar.gz",
        sha256 = "81156fece84ab6a9f2afdb109ce3ae577e42b1228441eded99bd77f627953b1a",
        strip_prefix = "crc32fast-1.2.1",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.crc32fast-1.2.1.bazel"),
    )

    maybe(
        http_archive,
        name = "raze__crossbeam_channel__0_5_1",
        url = "https://crates.io/api/v1/crates/crossbeam-channel/0.5.1/download",
        type = "tar.gz",
        sha256 = "06ed27e177f16d65f0f0c22a213e17c696ace5dd64b14258b52f9417ccb52db4",
        strip_prefix = "crossbeam-channel-0.5.1",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.crossbeam-channel-0.5.1.bazel"),
    )

    maybe(
        http_archive,
        name = "raze__crossbeam_deque__0_8_0",
        url = "https://crates.io/api/v1/crates/crossbeam-deque/0.8.0/download",
        type = "tar.gz",
        sha256 = "94af6efb46fef72616855b036a624cf27ba656ffc9be1b9a3c931cfc7749a9a9",
        strip_prefix = "crossbeam-deque-0.8.0",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.crossbeam-deque-0.8.0.bazel"),
    )

    maybe(
        http_archive,
        name = "raze__crossbeam_epoch__0_9_4",
        url = "https://crates.io/api/v1/crates/crossbeam-epoch/0.9.4/download",
        type = "tar.gz",
        sha256 = "52fb27eab85b17fbb9f6fd667089e07d6a2eb8743d02639ee7f6a7a7729c9c94",
        strip_prefix = "crossbeam-epoch-0.9.4",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.crossbeam-epoch-0.9.4.bazel"),
    )

    maybe(
        http_archive,
        name = "raze__crossbeam_utils__0_8_4",
        url = "https://crates.io/api/v1/crates/crossbeam-utils/0.8.4/download",
        type = "tar.gz",
        sha256 = "4feb231f0d4d6af81aed15928e58ecf5816aa62a2393e2c82f46973e92a9a278",
        strip_prefix = "crossbeam-utils-0.8.4",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.crossbeam-utils-0.8.4.bazel"),
    )

    maybe(
        http_archive,
        name = "raze__derive_new__0_5_9",
        url = "https://crates.io/api/v1/crates/derive-new/0.5.9/download",
        type = "tar.gz",
        sha256 = "3418329ca0ad70234b9735dc4ceed10af4df60eff9c8e7b06cb5e520d92c3535",
        strip_prefix = "derive-new-0.5.9",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.derive-new-0.5.9.bazel"),
    )

    maybe(
        http_archive,
        name = "raze__difference__2_0_0",
        url = "https://crates.io/api/v1/crates/difference/2.0.0/download",
        type = "tar.gz",
        sha256 = "524cbf6897b527295dff137cec09ecf3a05f4fddffd7dfcd1585403449e74198",
        strip_prefix = "difference-2.0.0",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.difference-2.0.0.bazel"),
    )

    maybe(
        http_archive,
        name = "raze__doc_comment__0_3_3",
        url = "https://crates.io/api/v1/crates/doc-comment/0.3.3/download",
        type = "tar.gz",
        sha256 = "fea41bba32d969b513997752735605054bc0dfa92b4c56bf1189f2e174be7a10",
        strip_prefix = "doc-comment-0.3.3",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.doc-comment-0.3.3.bazel"),
    )

    maybe(
        http_archive,
        name = "raze__either__1_6_1",
        url = "https://crates.io/api/v1/crates/either/1.6.1/download",
        type = "tar.gz",
        sha256 = "e78d4f1cc4ae33bbfc157ed5d5a5ef3bc29227303d595861deb238fcec4e9457",
        strip_prefix = "either-1.6.1",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.either-1.6.1.bazel"),
    )

    maybe(
        http_archive,
        name = "raze__flate2__1_0_20",
        url = "https://crates.io/api/v1/crates/flate2/1.0.20/download",
        type = "tar.gz",
        sha256 = "cd3aec53de10fe96d7d8c565eb17f2c687bb5518a2ec453b5b1252964526abe0",
        strip_prefix = "flate2-1.0.20",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.flate2-1.0.20.bazel"),
    )

    maybe(
        http_archive,
        name = "raze__float_cmp__0_8_0",
        url = "https://crates.io/api/v1/crates/float-cmp/0.8.0/download",
        type = "tar.gz",
        sha256 = "e1267f4ac4f343772758f7b1bdcbe767c218bbab93bb432acbf5162bbf85a6c4",
        strip_prefix = "float-cmp-0.8.0",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.float-cmp-0.8.0.bazel"),
    )

    maybe(
        http_archive,
        name = "raze__fst__0_3_5",
        url = "https://crates.io/api/v1/crates/fst/0.3.5/download",
        type = "tar.gz",
        sha256 = "927fb434ff9f0115b215dc0efd2e4fbdd7448522a92a1aa37c77d6a2f8f1ebd6",
        strip_prefix = "fst-0.3.5",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.fst-0.3.5.bazel"),
    )

    maybe(
        http_archive,
        name = "raze__fuchsia_cprng__0_1_1",
        url = "https://crates.io/api/v1/crates/fuchsia-cprng/0.1.1/download",
        type = "tar.gz",
        sha256 = "a06f77d526c1a601b7c4cdd98f54b5eaabffc14d5f2f0296febdc7f357c6d3ba",
        strip_prefix = "fuchsia-cprng-0.1.1",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.fuchsia-cprng-0.1.1.bazel"),
    )

    maybe(
        http_archive,
        name = "raze__gcc__0_3_55",
        url = "https://crates.io/api/v1/crates/gcc/0.3.55/download",
        type = "tar.gz",
        sha256 = "8f5f3913fa0bfe7ee1fd8248b6b9f42a5af4b9d65ec2dd2c3c26132b950ecfc2",
        strip_prefix = "gcc-0.3.55",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.gcc-0.3.55.bazel"),
    )

    maybe(
        http_archive,
        name = "raze__hermit_abi__0_1_18",
        url = "https://crates.io/api/v1/crates/hermit-abi/0.1.18/download",
        type = "tar.gz",
        sha256 = "322f4de77956e22ed0e5032c359a0f1273f1f7f0d79bfa3b8ffbc730d7fbcc5c",
        strip_prefix = "hermit-abi-0.1.18",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.hermit-abi-0.1.18.bazel"),
    )

    maybe(
        http_archive,
        name = "raze__itertools__0_8_2",
        url = "https://crates.io/api/v1/crates/itertools/0.8.2/download",
        type = "tar.gz",
        sha256 = "f56a2d0bc861f9165be4eb3442afd3c236d8a98afd426f65d92324ae1091a484",
        strip_prefix = "itertools-0.8.2",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.itertools-0.8.2.bazel"),
    )

    maybe(
        http_archive,
        name = "raze__itoa__0_4_7",
        url = "https://crates.io/api/v1/crates/itoa/0.4.7/download",
        type = "tar.gz",
        sha256 = "dd25036021b0de88a0aff6b850051563c6516d0bf53f8638938edbb9de732736",
        strip_prefix = "itoa-0.4.7",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.itoa-0.4.7.bazel"),
    )

    maybe(
        http_archive,
        name = "raze__json__0_11_15",
        url = "https://crates.io/api/v1/crates/json/0.11.15/download",
        type = "tar.gz",
        sha256 = "92c245af8786f6ac35f95ca14feca9119e71339aaab41e878e7cdd655c97e9e5",
        strip_prefix = "json-0.11.15",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.json-0.11.15.bazel"),
    )

    maybe(
        http_archive,
        name = "raze__lazy_static__1_4_0",
        url = "https://crates.io/api/v1/crates/lazy_static/1.4.0/download",
        type = "tar.gz",
        sha256 = "e2abad23fbc42b3700f2f279844dc832adb2b2eb069b2df918f455c4e18cc646",
        strip_prefix = "lazy_static-1.4.0",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.lazy_static-1.4.0.bazel"),
    )

    maybe(
        http_archive,
        name = "raze__libc__0_2_94",
        url = "https://crates.io/api/v1/crates/libc/0.2.94/download",
        type = "tar.gz",
        sha256 = "18794a8ad5b29321f790b55d93dfba91e125cb1a9edbd4f8e3150acc771c1a5e",
        strip_prefix = "libc-0.2.94",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.libc-0.2.94.bazel"),
    )

    maybe(
        http_archive,
        name = "raze__lock_api__0_3_4",
        url = "https://crates.io/api/v1/crates/lock_api/0.3.4/download",
        type = "tar.gz",
        sha256 = "c4da24a77a3d8a6d4862d95f72e6fdb9c09a643ecdb402d754004a557f2bec75",
        strip_prefix = "lock_api-0.3.4",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.lock_api-0.3.4.bazel"),
    )

    maybe(
        http_archive,
        name = "raze__log__0_4_14",
        url = "https://crates.io/api/v1/crates/log/0.4.14/download",
        type = "tar.gz",
        sha256 = "51b9bbe6c47d51fc3e1a9b945965946b4c44142ab8792c50835a980d362c2710",
        strip_prefix = "log-0.4.14",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.log-0.4.14.bazel"),
    )

    maybe(
        http_archive,
        name = "raze__memchr__2_4_0",
        url = "https://crates.io/api/v1/crates/memchr/2.4.0/download",
        type = "tar.gz",
        sha256 = "b16bd47d9e329435e309c58469fe0791c2d0d1ba96ec0954152a5ae2b04387dc",
        strip_prefix = "memchr-2.4.0",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.memchr-2.4.0.bazel"),
    )

    maybe(
        http_archive,
        name = "raze__memoffset__0_6_3",
        url = "https://crates.io/api/v1/crates/memoffset/0.6.3/download",
        type = "tar.gz",
        sha256 = "f83fb6581e8ed1f85fd45c116db8405483899489e38406156c25eb743554361d",
        strip_prefix = "memoffset-0.6.3",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.memoffset-0.6.3.bazel"),
    )

    maybe(
        http_archive,
        name = "raze__miniz_oxide__0_4_4",
        url = "https://crates.io/api/v1/crates/miniz_oxide/0.4.4/download",
        type = "tar.gz",
        sha256 = "a92518e98c078586bc6c934028adcca4c92a53d6a958196de835170a01d84e4b",
        strip_prefix = "miniz_oxide-0.4.4",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.miniz_oxide-0.4.4.bazel"),
    )

    maybe(
        http_archive,
        name = "raze__normalize_line_endings__0_3_0",
        url = "https://crates.io/api/v1/crates/normalize-line-endings/0.3.0/download",
        type = "tar.gz",
        sha256 = "61807f77802ff30975e01f4f071c8ba10c022052f98b3294119f3e615d13e5be",
        strip_prefix = "normalize-line-endings-0.3.0",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.normalize-line-endings-0.3.0.bazel"),
    )

    maybe(
        http_archive,
        name = "raze__num_traits__0_2_14",
        url = "https://crates.io/api/v1/crates/num-traits/0.2.14/download",
        type = "tar.gz",
        sha256 = "9a64b1ec5cda2586e284722486d802acf1f7dbdc623e2bfc57e65ca1cd099290",
        strip_prefix = "num-traits-0.2.14",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.num-traits-0.2.14.bazel"),
    )

    maybe(
        http_archive,
        name = "raze__num_cpus__1_13_0",
        url = "https://crates.io/api/v1/crates/num_cpus/1.13.0/download",
        type = "tar.gz",
        sha256 = "05499f3756671c15885fee9034446956fff3f243d6077b91e5767df161f766b3",
        strip_prefix = "num_cpus-1.13.0",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.num_cpus-1.13.0.bazel"),
    )

    maybe(
        http_archive,
        name = "raze__parking_lot__0_10_2",
        url = "https://crates.io/api/v1/crates/parking_lot/0.10.2/download",
        type = "tar.gz",
        sha256 = "d3a704eb390aafdc107b0e392f56a82b668e3a71366993b5340f5833fd62505e",
        strip_prefix = "parking_lot-0.10.2",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.parking_lot-0.10.2.bazel"),
    )

    maybe(
        http_archive,
        name = "raze__parking_lot_core__0_7_2",
        url = "https://crates.io/api/v1/crates/parking_lot_core/0.7.2/download",
        type = "tar.gz",
        sha256 = "d58c7c768d4ba344e3e8d72518ac13e259d7c7ade24167003b8488e10b6740a3",
        strip_prefix = "parking_lot_core-0.7.2",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.parking_lot_core-0.7.2.bazel"),
    )

    maybe(
        http_archive,
        name = "raze__pkg_config__0_3_19",
        url = "https://crates.io/api/v1/crates/pkg-config/0.3.19/download",
        type = "tar.gz",
        sha256 = "3831453b3449ceb48b6d9c7ad7c96d5ea673e9b470a1dc578c2ce6521230884c",
        strip_prefix = "pkg-config-0.3.19",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.pkg-config-0.3.19.bazel"),
    )

    maybe(
        http_archive,
        name = "raze__predicates__1_0_8",
        url = "https://crates.io/api/v1/crates/predicates/1.0.8/download",
        type = "tar.gz",
        sha256 = "f49cfaf7fdaa3bfacc6fa3e7054e65148878354a5cfddcf661df4c851f8021df",
        strip_prefix = "predicates-1.0.8",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.predicates-1.0.8.bazel"),
    )

    maybe(
        http_archive,
        name = "raze__predicates_core__1_0_2",
        url = "https://crates.io/api/v1/crates/predicates-core/1.0.2/download",
        type = "tar.gz",
        sha256 = "57e35a3326b75e49aa85f5dc6ec15b41108cf5aee58eabb1f274dd18b73c2451",
        strip_prefix = "predicates-core-1.0.2",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.predicates-core-1.0.2.bazel"),
    )

    maybe(
        http_archive,
        name = "raze__predicates_tree__1_0_2",
        url = "https://crates.io/api/v1/crates/predicates-tree/1.0.2/download",
        type = "tar.gz",
        sha256 = "15f553275e5721409451eb85e15fd9a860a6e5ab4496eb215987502b5f5391f2",
        strip_prefix = "predicates-tree-1.0.2",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.predicates-tree-1.0.2.bazel"),
    )

    maybe(
        http_archive,
        name = "raze__proc_macro2__1_0_26",
        url = "https://crates.io/api/v1/crates/proc-macro2/1.0.26/download",
        type = "tar.gz",
        sha256 = "a152013215dca273577e18d2bf00fa862b89b24169fb78c4c95aeb07992c9cec",
        strip_prefix = "proc-macro2-1.0.26",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.proc-macro2-1.0.26.bazel"),
    )

    maybe(
        http_archive,
        name = "raze__quick_error__1_2_3",
        url = "https://crates.io/api/v1/crates/quick-error/1.2.3/download",
        type = "tar.gz",
        sha256 = "a1d01941d82fa2ab50be1e79e6714289dd7cde78eba4c074bc5a4374f650dfe0",
        strip_prefix = "quick-error-1.2.3",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.quick-error-1.2.3.bazel"),
    )

    maybe(
        http_archive,
        name = "raze__quote__1_0_9",
        url = "https://crates.io/api/v1/crates/quote/1.0.9/download",
        type = "tar.gz",
        sha256 = "c3d0b9745dc2debf507c8422de05d7226cc1f0644216dfdfead988f9b1ab32a7",
        strip_prefix = "quote-1.0.9",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.quote-1.0.9.bazel"),
    )

    maybe(
        http_archive,
        name = "raze__rand__0_3_23",
        url = "https://crates.io/api/v1/crates/rand/0.3.23/download",
        type = "tar.gz",
        sha256 = "64ac302d8f83c0c1974bf758f6b041c6c8ada916fbb44a609158ca8b064cc76c",
        strip_prefix = "rand-0.3.23",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.rand-0.3.23.bazel"),
    )

    maybe(
        http_archive,
        name = "raze__rand__0_4_6",
        url = "https://crates.io/api/v1/crates/rand/0.4.6/download",
        type = "tar.gz",
        sha256 = "552840b97013b1a26992c11eac34bdd778e464601a4c2054b5f0bff7c6761293",
        strip_prefix = "rand-0.4.6",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.rand-0.4.6.bazel"),
    )

    maybe(
        http_archive,
        name = "raze__rand_core__0_3_1",
        url = "https://crates.io/api/v1/crates/rand_core/0.3.1/download",
        type = "tar.gz",
        sha256 = "7a6fdeb83b075e8266dcc8762c22776f6877a63111121f5f8c7411e5be7eed4b",
        strip_prefix = "rand_core-0.3.1",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.rand_core-0.3.1.bazel"),
    )

    maybe(
        http_archive,
        name = "raze__rand_core__0_4_2",
        url = "https://crates.io/api/v1/crates/rand_core/0.4.2/download",
        type = "tar.gz",
        sha256 = "9c33a3c44ca05fa6f1807d8e6743f3824e8509beca625669633be0acbdf509dc",
        strip_prefix = "rand_core-0.4.2",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.rand_core-0.4.2.bazel"),
    )

    maybe(
        http_archive,
        name = "raze__rayon__1_5_1",
        url = "https://crates.io/api/v1/crates/rayon/1.5.1/download",
        type = "tar.gz",
        sha256 = "c06aca804d41dbc8ba42dfd964f0d01334eceb64314b9ecf7c5fad5188a06d90",
        strip_prefix = "rayon-1.5.1",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.rayon-1.5.1.bazel"),
    )

    maybe(
        http_archive,
        name = "raze__rayon_core__1_9_1",
        url = "https://crates.io/api/v1/crates/rayon-core/1.9.1/download",
        type = "tar.gz",
        sha256 = "d78120e2c850279833f1dd3582f730c4ab53ed95aeaaaa862a2a5c71b1656d8e",
        strip_prefix = "rayon-core-1.9.1",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.rayon-core-1.9.1.bazel"),
    )

    maybe(
        http_archive,
        name = "raze__rdrand__0_4_0",
        url = "https://crates.io/api/v1/crates/rdrand/0.4.0/download",
        type = "tar.gz",
        sha256 = "678054eb77286b51581ba43620cc911abf02758c91f93f479767aed0f90458b2",
        strip_prefix = "rdrand-0.4.0",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.rdrand-0.4.0.bazel"),
    )

    maybe(
        http_archive,
        name = "raze__redox_syscall__0_1_57",
        url = "https://crates.io/api/v1/crates/redox_syscall/0.1.57/download",
        type = "tar.gz",
        sha256 = "41cc0f7e4d5d4544e8861606a285bb08d3e70712ccc7d2b84d7c0ccfaf4b05ce",
        strip_prefix = "redox_syscall-0.1.57",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.redox_syscall-0.1.57.bazel"),
    )

    maybe(
        http_archive,
        name = "raze__regex__1_5_4",
        url = "https://crates.io/api/v1/crates/regex/1.5.4/download",
        type = "tar.gz",
        sha256 = "d07a8629359eb56f1e2fb1652bb04212c072a87ba68546a04065d525673ac461",
        strip_prefix = "regex-1.5.4",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.regex-1.5.4.bazel"),
    )

    maybe(
        http_archive,
        name = "raze__regex_automata__0_1_9",
        url = "https://crates.io/api/v1/crates/regex-automata/0.1.9/download",
        type = "tar.gz",
        sha256 = "ae1ded71d66a4a97f5e961fd0cb25a5f366a42a41570d16a763a69c092c26ae4",
        strip_prefix = "regex-automata-0.1.9",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.regex-automata-0.1.9.bazel"),
    )

    maybe(
        http_archive,
        name = "raze__regex_syntax__0_6_25",
        url = "https://crates.io/api/v1/crates/regex-syntax/0.6.25/download",
        type = "tar.gz",
        sha256 = "f497285884f3fcff424ffc933e56d7cbca511def0c9831a7f9b5f6153e3cc89b",
        strip_prefix = "regex-syntax-0.6.25",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.regex-syntax-0.6.25.bazel"),
    )

    maybe(
        http_archive,
        name = "raze__remove_dir_all__0_5_3",
        url = "https://crates.io/api/v1/crates/remove_dir_all/0.5.3/download",
        type = "tar.gz",
        sha256 = "3acd125665422973a33ac9d3dd2df85edad0f4ae9b00dafb1a05e43a9f5ef8e7",
        strip_prefix = "remove_dir_all-0.5.3",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.remove_dir_all-0.5.3.bazel"),
    )

    maybe(
        http_archive,
        name = "raze__rls_analysis__0_18_1",
        url = "https://crates.io/api/v1/crates/rls-analysis/0.18.1/download",
        type = "tar.gz",
        sha256 = "534032993e1b60e5db934eab2dde54da7afd1e46c3465fddb2b29eb47cb1ed3a",
        strip_prefix = "rls-analysis-0.18.1",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.rls-analysis-0.18.1.bazel"),
    )

    maybe(
        http_archive,
        name = "raze__rls_data__0_19_1",
        url = "https://crates.io/api/v1/crates/rls-data/0.19.1/download",
        type = "tar.gz",
        sha256 = "a58135eb039f3a3279a33779192f0ee78b56f57ae636e25cec83530e41debb99",
        strip_prefix = "rls-data-0.19.1",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.rls-data-0.19.1.bazel"),
    )

    maybe(
        http_archive,
        name = "raze__rls_span__0_5_3",
        url = "https://crates.io/api/v1/crates/rls-span/0.5.3/download",
        type = "tar.gz",
        sha256 = "f0eea58478fc06e15f71b03236612173a1b81e9770314edecfa664375e3e4c86",
        strip_prefix = "rls-span-0.5.3",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.rls-span-0.5.3.bazel"),
    )

    maybe(
        http_archive,
        name = "raze__rust_crypto__0_2_36",
        url = "https://crates.io/api/v1/crates/rust-crypto/0.2.36/download",
        type = "tar.gz",
        sha256 = "f76d05d3993fd5f4af9434e8e436db163a12a9d40e1a58a726f27a01dfd12a2a",
        strip_prefix = "rust-crypto-0.2.36",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.rust-crypto-0.2.36.bazel"),
    )

    maybe(
        http_archive,
        name = "raze__rustc_serialize__0_3_24",
        url = "https://crates.io/api/v1/crates/rustc-serialize/0.3.24/download",
        type = "tar.gz",
        sha256 = "dcf128d1287d2ea9d80910b5f1120d0b8eede3fbf1abe91c40d39ea7d51e6fda",
        strip_prefix = "rustc-serialize-0.3.24",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.rustc-serialize-0.3.24.bazel"),
    )

    maybe(
        http_archive,
        name = "raze__ryu__1_0_5",
        url = "https://crates.io/api/v1/crates/ryu/1.0.5/download",
        type = "tar.gz",
        sha256 = "71d301d4193d031abdd79ff7e3dd721168a9572ef3fe51a1517aba235bd8f86e",
        strip_prefix = "ryu-1.0.5",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.ryu-1.0.5.bazel"),
    )

    maybe(
        http_archive,
        name = "raze__scopeguard__1_1_0",
        url = "https://crates.io/api/v1/crates/scopeguard/1.1.0/download",
        type = "tar.gz",
        sha256 = "d29ab0c6d3fc0ee92fe66e2d99f700eab17a8d57d1c1d3b748380fb20baa78cd",
        strip_prefix = "scopeguard-1.1.0",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.scopeguard-1.1.0.bazel"),
    )

    maybe(
        http_archive,
        name = "raze__serde__1_0_126",
        url = "https://crates.io/api/v1/crates/serde/1.0.126/download",
        type = "tar.gz",
        sha256 = "ec7505abeacaec74ae4778d9d9328fe5a5d04253220a85c4ee022239fc996d03",
        strip_prefix = "serde-1.0.126",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.serde-1.0.126.bazel"),
    )

    maybe(
        http_archive,
        name = "raze__serde_derive__1_0_126",
        url = "https://crates.io/api/v1/crates/serde_derive/1.0.126/download",
        type = "tar.gz",
        sha256 = "963a7dbc9895aeac7ac90e74f34a5d5261828f79df35cbed41e10189d3804d43",
        strip_prefix = "serde_derive-1.0.126",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.serde_derive-1.0.126.bazel"),
    )

    maybe(
        http_archive,
        name = "raze__serde_json__1_0_64",
        url = "https://crates.io/api/v1/crates/serde_json/1.0.64/download",
        type = "tar.gz",
        sha256 = "799e97dc9fdae36a5c8b8f2cae9ce2ee9fdce2058c57a93e6099d919fd982f79",
        strip_prefix = "serde_json-1.0.64",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.serde_json-1.0.64.bazel"),
    )

    maybe(
        http_archive,
        name = "raze__serial_test__0_4_0",
        url = "https://crates.io/api/v1/crates/serial_test/0.4.0/download",
        type = "tar.gz",
        sha256 = "fef5f7c7434b2f2c598adc6f9494648a1e41274a75c0ba4056f680ae0c117fd6",
        strip_prefix = "serial_test-0.4.0",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.serial_test-0.4.0.bazel"),
    )

    maybe(
        http_archive,
        name = "raze__serial_test_derive__0_4_0",
        url = "https://crates.io/api/v1/crates/serial_test_derive/0.4.0/download",
        type = "tar.gz",
        sha256 = "d08338d8024b227c62bd68a12c7c9883f5c66780abaef15c550dc56f46ee6515",
        strip_prefix = "serial_test_derive-0.4.0",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.serial_test_derive-0.4.0.bazel"),
    )

    maybe(
        http_archive,
        name = "raze__smallvec__1_6_1",
        url = "https://crates.io/api/v1/crates/smallvec/1.6.1/download",
        type = "tar.gz",
        sha256 = "fe0f37c9e8f3c5a4a66ad655a93c74daac4ad00c441533bf5c6e7990bb42604e",
        strip_prefix = "smallvec-1.6.1",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.smallvec-1.6.1.bazel"),
    )

    maybe(
        http_archive,
        name = "raze__strsim__0_8_0",
        url = "https://crates.io/api/v1/crates/strsim/0.8.0/download",
        type = "tar.gz",
        sha256 = "8ea5119cdb4c55b55d432abb513a0429384878c15dde60cc77b1c99de1a95a6a",
        strip_prefix = "strsim-0.8.0",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.strsim-0.8.0.bazel"),
    )

    maybe(
        http_archive,
        name = "raze__syn__1_0_72",
        url = "https://crates.io/api/v1/crates/syn/1.0.72/download",
        type = "tar.gz",
        sha256 = "a1e8cdbefb79a9a5a65e0db8b47b723ee907b7c7f8496c76a1770b5c310bab82",
        strip_prefix = "syn-1.0.72",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.syn-1.0.72.bazel"),
    )

    maybe(
        http_archive,
        name = "raze__tempdir__0_3_7",
        url = "https://crates.io/api/v1/crates/tempdir/0.3.7/download",
        type = "tar.gz",
        sha256 = "15f2b5fb00ccdf689e0149d1b1b3c03fead81c2b37735d812fa8bddbbf41b6d8",
        strip_prefix = "tempdir-0.3.7",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.tempdir-0.3.7.bazel"),
    )

    maybe(
        http_archive,
        name = "raze__textwrap__0_11_0",
        url = "https://crates.io/api/v1/crates/textwrap/0.11.0/download",
        type = "tar.gz",
        sha256 = "d326610f408c7a4eb6f51c37c330e496b08506c9457c9d34287ecc38809fb060",
        strip_prefix = "textwrap-0.11.0",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.textwrap-0.11.0.bazel"),
    )

    maybe(
        http_archive,
        name = "raze__thiserror__1_0_24",
        url = "https://crates.io/api/v1/crates/thiserror/1.0.24/download",
        type = "tar.gz",
        sha256 = "e0f4a65597094d4483ddaed134f409b2cb7c1beccf25201a9f73c719254fa98e",
        strip_prefix = "thiserror-1.0.24",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.thiserror-1.0.24.bazel"),
    )

    maybe(
        http_archive,
        name = "raze__thiserror_impl__1_0_24",
        url = "https://crates.io/api/v1/crates/thiserror-impl/1.0.24/download",
        type = "tar.gz",
        sha256 = "7765189610d8241a44529806d6fd1f2e0a08734313a35d5b3a556f92b381f3c0",
        strip_prefix = "thiserror-impl-1.0.24",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.thiserror-impl-1.0.24.bazel"),
    )

    maybe(
        http_archive,
        name = "raze__time__0_1_44",
        url = "https://crates.io/api/v1/crates/time/0.1.44/download",
        type = "tar.gz",
        sha256 = "6db9e6914ab8b1ae1c260a4ae7a49b6c5611b40328a735b21862567685e73255",
        strip_prefix = "time-0.1.44",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.time-0.1.44.bazel"),
    )

    maybe(
        http_archive,
        name = "raze__treeline__0_1_0",
        url = "https://crates.io/api/v1/crates/treeline/0.1.0/download",
        type = "tar.gz",
        sha256 = "a7f741b240f1a48843f9b8e0444fb55fb2a4ff67293b50a9179dfd5ea67f8d41",
        strip_prefix = "treeline-0.1.0",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.treeline-0.1.0.bazel"),
    )

    maybe(
        http_archive,
        name = "raze__unicode_width__0_1_8",
        url = "https://crates.io/api/v1/crates/unicode-width/0.1.8/download",
        type = "tar.gz",
        sha256 = "9337591893a19b88d8d87f2cec1e73fad5cdfd10e5a6f349f498ad6ea2ffb1e3",
        strip_prefix = "unicode-width-0.1.8",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.unicode-width-0.1.8.bazel"),
    )

    maybe(
        http_archive,
        name = "raze__unicode_xid__0_2_2",
        url = "https://crates.io/api/v1/crates/unicode-xid/0.2.2/download",
        type = "tar.gz",
        sha256 = "8ccb82d61f80a663efe1f787a51b16b5a51e3314d6ac365b08639f52387b33f3",
        strip_prefix = "unicode-xid-0.2.2",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.unicode-xid-0.2.2.bazel"),
    )

    maybe(
        http_archive,
        name = "raze__vec_map__0_8_2",
        url = "https://crates.io/api/v1/crates/vec_map/0.8.2/download",
        type = "tar.gz",
        sha256 = "f1bddf1187be692e79c5ffeab891132dfb0f236ed36a43c7ed39f1165ee20191",
        strip_prefix = "vec_map-0.8.2",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.vec_map-0.8.2.bazel"),
    )

    maybe(
        http_archive,
        name = "raze__wait_timeout__0_2_0",
        url = "https://crates.io/api/v1/crates/wait-timeout/0.2.0/download",
        type = "tar.gz",
        sha256 = "9f200f5b12eb75f8c1ed65abd4b2db8a6e1b138a20de009dacee265a2498f3f6",
        strip_prefix = "wait-timeout-0.2.0",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.wait-timeout-0.2.0.bazel"),
    )

    maybe(
        http_archive,
        name = "raze__wasi__0_10_0_wasi_snapshot_preview1",
        url = "https://crates.io/api/v1/crates/wasi/0.10.0+wasi-snapshot-preview1/download",
        type = "tar.gz",
        sha256 = "1a143597ca7c7793eff794def352d41792a93c481eb1042423ff7ff72ba2c31f",
        strip_prefix = "wasi-0.10.0+wasi-snapshot-preview1",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.wasi-0.10.0+wasi-snapshot-preview1.bazel"),
    )

    maybe(
        http_archive,
        name = "raze__winapi__0_3_9",
        url = "https://crates.io/api/v1/crates/winapi/0.3.9/download",
        type = "tar.gz",
        sha256 = "5c839a674fcd7a98952e593242ea400abe93992746761e38641405d28b00f419",
        strip_prefix = "winapi-0.3.9",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.winapi-0.3.9.bazel"),
    )

    maybe(
        http_archive,
        name = "raze__winapi_i686_pc_windows_gnu__0_4_0",
        url = "https://crates.io/api/v1/crates/winapi-i686-pc-windows-gnu/0.4.0/download",
        type = "tar.gz",
        sha256 = "ac3b87c63620426dd9b991e5ce0329eff545bccbbb34f3be09ff6fb6ab51b7b6",
        strip_prefix = "winapi-i686-pc-windows-gnu-0.4.0",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.winapi-i686-pc-windows-gnu-0.4.0.bazel"),
    )

    maybe(
        http_archive,
        name = "raze__winapi_x86_64_pc_windows_gnu__0_4_0",
        url = "https://crates.io/api/v1/crates/winapi-x86_64-pc-windows-gnu/0.4.0/download",
        type = "tar.gz",
        sha256 = "712e227841d057c1ee1cd2fb22fa7e5a5461ae8e48fa2ca79ec42cfc1931183f",
        strip_prefix = "winapi-x86_64-pc-windows-gnu-0.4.0",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.winapi-x86_64-pc-windows-gnu-0.4.0.bazel"),
    )

    maybe(
        http_archive,
        name = "raze__zip__0_5_12",
        url = "https://crates.io/api/v1/crates/zip/0.5.12/download",
        type = "tar.gz",
        sha256 = "9c83dc9b784d252127720168abd71ea82bf8c3d96b17dc565b5e2a02854f2b27",
        strip_prefix = "zip-0.5.12",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.zip-0.5.12.bazel"),
    )
