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
        name = "raze__adler__0_2_3",
        url = "https://crates.io/api/v1/crates/adler/0.2.3/download",
        type = "tar.gz",
        sha256 = "ee2a4ec343196209d6594e19543ae87a39f96d5534d7174822a3ad825dd6ed7e",
        strip_prefix = "adler-0.2.3",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.adler-0.2.3.bazel"),
    )

    maybe(
        http_archive,
        name = "raze__aho_corasick__0_7_13",
        url = "https://crates.io/api/v1/crates/aho-corasick/0.7.13/download",
        type = "tar.gz",
        sha256 = "043164d8ba5c4c3035fec9bbee8647c0261d788f3474306f93bb65901cae0e86",
        strip_prefix = "aho-corasick-0.7.13",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.aho-corasick-0.7.13.bazel"),
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
        name = "raze__anyhow__1_0_32",
        url = "https://crates.io/api/v1/crates/anyhow/1.0.32/download",
        type = "tar.gz",
        sha256 = "6b602bfe940d21c130f3895acd65221e8a61270debe89d628b9cb4e3ccb8569b",
        strip_prefix = "anyhow-1.0.32",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.anyhow-1.0.32.bazel"),
    )

    maybe(
        http_archive,
        name = "raze__assert_cmd__1_0_1",
        url = "https://crates.io/api/v1/crates/assert_cmd/1.0.1/download",
        type = "tar.gz",
        sha256 = "c88b9ca26f9c16ec830350d309397e74ee9abdfd8eb1f71cb6ecc71a3fc818da",
        strip_prefix = "assert_cmd-1.0.1",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.assert_cmd-1.0.1.bazel"),
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
        name = "raze__autocfg__1_0_0",
        url = "https://crates.io/api/v1/crates/autocfg/1.0.0/download",
        type = "tar.gz",
        sha256 = "f8aac770f1885fd7e387acedd76065302551364496e46b3dd00860b2f8359b9d",
        strip_prefix = "autocfg-1.0.0",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.autocfg-1.0.0.bazel"),
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
        name = "raze__byteorder__1_3_4",
        url = "https://crates.io/api/v1/crates/byteorder/1.3.4/download",
        type = "tar.gz",
        sha256 = "08c48aae112d48ed9f069b33538ea9e3e90aa263cfa3d1c24309612b1f7472de",
        strip_prefix = "byteorder-1.3.4",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.byteorder-1.3.4.bazel"),
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
        name = "raze__bzip2_sys__0_1_9_1_0_8",
        url = "https://crates.io/api/v1/crates/bzip2-sys/0.1.9+1.0.8/download",
        type = "tar.gz",
        sha256 = "ad3b39a260062fca31f7b0b12f207e8f2590a67d32ec7d59c20484b07ea7285e",
        strip_prefix = "bzip2-sys-0.1.9+1.0.8",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.bzip2-sys-0.1.9+1.0.8.bazel"),
    )

    maybe(
        http_archive,
        name = "raze__cc__1_0_58",
        url = "https://crates.io/api/v1/crates/cc/1.0.58/download",
        type = "tar.gz",
        sha256 = "f9a06fb2e53271d7c279ec1efea6ab691c35a2ae67ec0d91d7acec0caf13b518",
        strip_prefix = "cc-1.0.58",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.cc-1.0.58.bazel"),
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
        strip_prefix = "cfg-if-1.0.0",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.cfg-if-1.0.0.bazel"),
    )

    maybe(
        http_archive,
        name = "raze__clap__2_33_1",
        url = "https://crates.io/api/v1/crates/clap/2.33.1/download",
        type = "tar.gz",
        sha256 = "bdfa80d47f954d53a35a64987ca1422f495b8d6483c0fe9f7117b36c2a792129",
        strip_prefix = "clap-2.33.1",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.clap-2.33.1.bazel"),
    )

    maybe(
        http_archive,
        name = "raze__cloudabi__0_0_3",
        url = "https://crates.io/api/v1/crates/cloudabi/0.0.3/download",
        type = "tar.gz",
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
        name = "raze__const_fn__0_4_4",
        url = "https://crates.io/api/v1/crates/const_fn/0.4.4/download",
        type = "tar.gz",
        strip_prefix = "const_fn-0.4.4",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.const_fn-0.4.4.bazel"),
    )

    maybe(
        http_archive,
        name = "raze__crc32fast__1_2_0",
        url = "https://crates.io/api/v1/crates/crc32fast/1.2.0/download",
        type = "tar.gz",
        sha256 = "ba125de2af0df55319f41944744ad91c71113bf74a4646efff39afe1f6842db1",
        strip_prefix = "crc32fast-1.2.0",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.crc32fast-1.2.0.bazel"),
    )

    maybe(
        http_archive,
        name = "raze__crossbeam_channel__0_5_0",
        url = "https://crates.io/api/v1/crates/crossbeam-channel/0.5.0/download",
        type = "tar.gz",
        strip_prefix = "crossbeam-channel-0.5.0",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.crossbeam-channel-0.5.0.bazel"),
    )

    maybe(
        http_archive,
        name = "raze__crossbeam_deque__0_8_0",
        url = "https://crates.io/api/v1/crates/crossbeam-deque/0.8.0/download",
        type = "tar.gz",
        strip_prefix = "crossbeam-deque-0.8.0",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.crossbeam-deque-0.8.0.bazel"),
    )

    maybe(
        http_archive,
        name = "raze__crossbeam_epoch__0_9_1",
        url = "https://crates.io/api/v1/crates/crossbeam-epoch/0.9.1/download",
        type = "tar.gz",
        strip_prefix = "crossbeam-epoch-0.9.1",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.crossbeam-epoch-0.9.1.bazel"),
    )

    maybe(
        http_archive,
        name = "raze__crossbeam_utils__0_8_1",
        url = "https://crates.io/api/v1/crates/crossbeam-utils/0.8.1/download",
        type = "tar.gz",
        strip_prefix = "crossbeam-utils-0.8.1",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.crossbeam-utils-0.8.1.bazel"),
    )

    maybe(
        http_archive,
        name = "raze__derive_new__0_5_8",
        url = "https://crates.io/api/v1/crates/derive-new/0.5.8/download",
        type = "tar.gz",
        sha256 = "71f31892cd5c62e414316f2963c5689242c43d8e7bbcaaeca97e5e28c95d91d9",
        strip_prefix = "derive-new-0.5.8",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.derive-new-0.5.8.bazel"),
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
        name = "raze__either__1_5_3",
        url = "https://crates.io/api/v1/crates/either/1.5.3/download",
        type = "tar.gz",
        sha256 = "bb1f6b1ce1c140482ea30ddd3335fc0024ac7ee112895426e0a629a6c20adfe3",
        strip_prefix = "either-1.5.3",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.either-1.5.3.bazel"),
    )

    maybe(
        http_archive,
        name = "raze__flate2__1_0_16",
        url = "https://crates.io/api/v1/crates/flate2/1.0.16/download",
        type = "tar.gz",
        sha256 = "68c90b0fc46cf89d227cc78b40e494ff81287a92dd07631e5af0d06fe3cf885e",
        strip_prefix = "flate2-1.0.16",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.flate2-1.0.16.bazel"),
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
        name = "raze__hermit_abi__0_1_15",
        url = "https://crates.io/api/v1/crates/hermit-abi/0.1.15/download",
        type = "tar.gz",
        sha256 = "3deed196b6e7f9e44a2ae8d94225d80302d81208b1bb673fd21fe634645c85a9",
        strip_prefix = "hermit-abi-0.1.15",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.hermit-abi-0.1.15.bazel"),
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
        name = "raze__itoa__0_4_6",
        url = "https://crates.io/api/v1/crates/itoa/0.4.6/download",
        type = "tar.gz",
        sha256 = "dc6f3ad7b9d11a0c00842ff8de1b60ee58661048eb8049ed33c73594f359d7e6",
        strip_prefix = "itoa-0.4.6",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.itoa-0.4.6.bazel"),
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
        name = "raze__libc__0_2_74",
        url = "https://crates.io/api/v1/crates/libc/0.2.74/download",
        type = "tar.gz",
        sha256 = "a2f02823cf78b754822df5f7f268fb59822e7296276d3e069d8e8cb26a14bd10",
        strip_prefix = "libc-0.2.74",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.libc-0.2.74.bazel"),
    )

    maybe(
        http_archive,
        name = "raze__lock_api__0_3_4",
        url = "https://crates.io/api/v1/crates/lock_api/0.3.4/download",
        type = "tar.gz",
        strip_prefix = "lock_api-0.3.4",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.lock_api-0.3.4.bazel"),
    )

    maybe(
        http_archive,
        name = "raze__log__0_4_11",
        url = "https://crates.io/api/v1/crates/log/0.4.11/download",
        type = "tar.gz",
        sha256 = "4fabed175da42fed1fa0746b0ea71f412aa9d35e76e95e59b192c64b9dc2bf8b",
        strip_prefix = "log-0.4.11",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.log-0.4.11.bazel"),
    )

    maybe(
        http_archive,
        name = "raze__memchr__2_3_3",
        url = "https://crates.io/api/v1/crates/memchr/2.3.3/download",
        type = "tar.gz",
        sha256 = "3728d817d99e5ac407411fa471ff9800a778d88a24685968b36824eaf4bee400",
        strip_prefix = "memchr-2.3.3",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.memchr-2.3.3.bazel"),
    )

    maybe(
        http_archive,
        name = "raze__memoffset__0_6_1",
        url = "https://crates.io/api/v1/crates/memoffset/0.6.1/download",
        type = "tar.gz",
        strip_prefix = "memoffset-0.6.1",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.memoffset-0.6.1.bazel"),
    )

    maybe(
        http_archive,
        name = "raze__miniz_oxide__0_4_0",
        url = "https://crates.io/api/v1/crates/miniz_oxide/0.4.0/download",
        type = "tar.gz",
        sha256 = "be0f75932c1f6cfae3c04000e40114adf955636e19040f9c0a2c380702aa1c7f",
        strip_prefix = "miniz_oxide-0.4.0",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.miniz_oxide-0.4.0.bazel"),
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
        name = "raze__num_traits__0_2_12",
        url = "https://crates.io/api/v1/crates/num-traits/0.2.12/download",
        type = "tar.gz",
        sha256 = "ac267bcc07f48ee5f8935ab0d24f316fb722d7a1292e2913f0cc196b29ffd611",
        strip_prefix = "num-traits-0.2.12",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.num-traits-0.2.12.bazel"),
    )

    maybe(
        http_archive,
        name = "raze__num_cpus__1_13_0",
        url = "https://crates.io/api/v1/crates/num_cpus/1.13.0/download",
        type = "tar.gz",
        strip_prefix = "num_cpus-1.13.0",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.num_cpus-1.13.0.bazel"),
    )

    maybe(
        http_archive,
        name = "raze__parking_lot__0_10_2",
        url = "https://crates.io/api/v1/crates/parking_lot/0.10.2/download",
        type = "tar.gz",
        strip_prefix = "parking_lot-0.10.2",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.parking_lot-0.10.2.bazel"),
    )

    maybe(
        http_archive,
        name = "raze__parking_lot_core__0_7_2",
        url = "https://crates.io/api/v1/crates/parking_lot_core/0.7.2/download",
        type = "tar.gz",
        strip_prefix = "parking_lot_core-0.7.2",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.parking_lot_core-0.7.2.bazel"),
    )

    maybe(
        http_archive,
        name = "raze__pkg_config__0_3_18",
        url = "https://crates.io/api/v1/crates/pkg-config/0.3.18/download",
        type = "tar.gz",
        sha256 = "d36492546b6af1463394d46f0c834346f31548646f6ba10849802c9c9a27ac33",
        strip_prefix = "pkg-config-0.3.18",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.pkg-config-0.3.18.bazel"),
    )

    maybe(
        http_archive,
        name = "raze__podio__0_1_7",
        url = "https://crates.io/api/v1/crates/podio/0.1.7/download",
        type = "tar.gz",
        sha256 = "b18befed8bc2b61abc79a457295e7e838417326da1586050b919414073977f19",
        strip_prefix = "podio-0.1.7",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.podio-0.1.7.bazel"),
    )

    maybe(
        http_archive,
        name = "raze__predicates__1_0_5",
        url = "https://crates.io/api/v1/crates/predicates/1.0.5/download",
        type = "tar.gz",
        sha256 = "96bfead12e90dccead362d62bb2c90a5f6fc4584963645bc7f71a735e0b0735a",
        strip_prefix = "predicates-1.0.5",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.predicates-1.0.5.bazel"),
    )

    maybe(
        http_archive,
        name = "raze__predicates_core__1_0_0",
        url = "https://crates.io/api/v1/crates/predicates-core/1.0.0/download",
        type = "tar.gz",
        sha256 = "06075c3a3e92559ff8929e7a280684489ea27fe44805174c3ebd9328dcb37178",
        strip_prefix = "predicates-core-1.0.0",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.predicates-core-1.0.0.bazel"),
    )

    maybe(
        http_archive,
        name = "raze__predicates_tree__1_0_0",
        url = "https://crates.io/api/v1/crates/predicates-tree/1.0.0/download",
        type = "tar.gz",
        sha256 = "8e63c4859013b38a76eca2414c64911fba30def9e3202ac461a2d22831220124",
        strip_prefix = "predicates-tree-1.0.0",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.predicates-tree-1.0.0.bazel"),
    )

    maybe(
        http_archive,
        name = "raze__proc_macro2__1_0_19",
        url = "https://crates.io/api/v1/crates/proc-macro2/1.0.19/download",
        type = "tar.gz",
        sha256 = "04f5f085b5d71e2188cb8271e5da0161ad52c3f227a661a3c135fdf28e258b12",
        strip_prefix = "proc-macro2-1.0.19",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.proc-macro2-1.0.19.bazel"),
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
        name = "raze__quote__1_0_7",
        url = "https://crates.io/api/v1/crates/quote/1.0.7/download",
        type = "tar.gz",
        sha256 = "aa563d17ecb180e500da1cfd2b028310ac758de548efdd203e18f283af693f37",
        strip_prefix = "quote-1.0.7",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.quote-1.0.7.bazel"),
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
        name = "raze__rayon__1_5_0",
        url = "https://crates.io/api/v1/crates/rayon/1.5.0/download",
        type = "tar.gz",
        strip_prefix = "rayon-1.5.0",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.rayon-1.5.0.bazel"),
    )

    maybe(
        http_archive,
        name = "raze__rayon_core__1_9_0",
        url = "https://crates.io/api/v1/crates/rayon-core/1.9.0/download",
        type = "tar.gz",
        strip_prefix = "rayon-core-1.9.0",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.rayon-core-1.9.0.bazel"),
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
        strip_prefix = "redox_syscall-0.1.57",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.redox_syscall-0.1.57.bazel"),
    )

    maybe(
        http_archive,
        name = "raze__regex__1_3_9",
        url = "https://crates.io/api/v1/crates/regex/1.3.9/download",
        type = "tar.gz",
        sha256 = "9c3780fcf44b193bc4d09f36d2a3c87b251da4a046c87795a0d35f4f927ad8e6",
        strip_prefix = "regex-1.3.9",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.regex-1.3.9.bazel"),
    )

    maybe(
        http_archive,
        name = "raze__regex_syntax__0_6_18",
        url = "https://crates.io/api/v1/crates/regex-syntax/0.6.18/download",
        type = "tar.gz",
        sha256 = "26412eb97c6b088a6997e05f69403a802a92d520de2f8e63c2b65f9e0f47c4e8",
        strip_prefix = "regex-syntax-0.6.18",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.regex-syntax-0.6.18.bazel"),
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
        name = "raze__rls_data__0_19_0",
        url = "https://crates.io/api/v1/crates/rls-data/0.19.0/download",
        type = "tar.gz",
        sha256 = "76c72ea97e045be5f6290bb157ebdc5ee9f2b093831ff72adfaf59025cf5c491",
        strip_prefix = "rls-data-0.19.0",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.rls-data-0.19.0.bazel"),
    )

    maybe(
        http_archive,
        name = "raze__rls_span__0_5_2",
        url = "https://crates.io/api/v1/crates/rls-span/0.5.2/download",
        type = "tar.gz",
        sha256 = "f2e9bed56f6272bd85d9d06d1aaeef80c5fddc78a82199eb36dceb5f94e7d934",
        strip_prefix = "rls-span-0.5.2",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.rls-span-0.5.2.bazel"),
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
        strip_prefix = "scopeguard-1.1.0",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.scopeguard-1.1.0.bazel"),
    )

    maybe(
        http_archive,
        name = "raze__serde__1_0_114",
        url = "https://crates.io/api/v1/crates/serde/1.0.114/download",
        type = "tar.gz",
        sha256 = "5317f7588f0a5078ee60ef675ef96735a1442132dc645eb1d12c018620ed8cd3",
        strip_prefix = "serde-1.0.114",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.serde-1.0.114.bazel"),
    )

    maybe(
        http_archive,
        name = "raze__serde_json__1_0_57",
        url = "https://crates.io/api/v1/crates/serde_json/1.0.57/download",
        type = "tar.gz",
        sha256 = "164eacbdb13512ec2745fb09d51fd5b22b0d65ed294a1dcf7285a360c80a675c",
        strip_prefix = "serde_json-1.0.57",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.serde_json-1.0.57.bazel"),
    )

    maybe(
        http_archive,
        name = "raze__serial_test__0_4_0",
        url = "https://crates.io/api/v1/crates/serial_test/0.4.0/download",
        type = "tar.gz",
        strip_prefix = "serial_test-0.4.0",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.serial_test-0.4.0.bazel"),
    )

    maybe(
        http_archive,
        name = "raze__serial_test_derive__0_4_0",
        url = "https://crates.io/api/v1/crates/serial_test_derive/0.4.0/download",
        type = "tar.gz",
        strip_prefix = "serial_test_derive-0.4.0",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.serial_test_derive-0.4.0.bazel"),
    )

    maybe(
        http_archive,
        name = "raze__smallvec__1_5_1",
        url = "https://crates.io/api/v1/crates/smallvec/1.5.1/download",
        type = "tar.gz",
        strip_prefix = "smallvec-1.5.1",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.smallvec-1.5.1.bazel"),
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
        name = "raze__syn__1_0_36",
        url = "https://crates.io/api/v1/crates/syn/1.0.36/download",
        type = "tar.gz",
        sha256 = "4cdb98bcb1f9d81d07b536179c269ea15999b5d14ea958196413869445bb5250",
        strip_prefix = "syn-1.0.36",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.syn-1.0.36.bazel"),
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
        name = "raze__thread_local__1_0_1",
        url = "https://crates.io/api/v1/crates/thread_local/1.0.1/download",
        type = "tar.gz",
        sha256 = "d40c6d1b69745a6ec6fb1ca717914848da4b44ae29d9b3080cbee91d72a69b14",
        strip_prefix = "thread_local-1.0.1",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.thread_local-1.0.1.bazel"),
    )

    maybe(
        http_archive,
        name = "raze__time__0_1_43",
        url = "https://crates.io/api/v1/crates/time/0.1.43/download",
        type = "tar.gz",
        sha256 = "ca8a50ef2360fbd1eeb0ecd46795a87a19024eb4b53c5dc916ca1fd95fe62438",
        strip_prefix = "time-0.1.43",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.time-0.1.43.bazel"),
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
        name = "raze__unicode_xid__0_2_1",
        url = "https://crates.io/api/v1/crates/unicode-xid/0.2.1/download",
        type = "tar.gz",
        sha256 = "f7fe0bb3479651439c9112f72b6c505038574c9fbb575ed1bf3b797fa39dd564",
        strip_prefix = "unicode-xid-0.2.1",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.unicode-xid-0.2.1.bazel"),
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
        name = "raze__zip__0_5_6",
        url = "https://crates.io/api/v1/crates/zip/0.5.6/download",
        type = "tar.gz",
        sha256 = "58287c28d78507f5f91f2a4cf1e8310e2c76fd4c6932f93ac60fd1ceb402db7d",
        strip_prefix = "zip-0.5.6",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.zip-0.5.6.bazel"),
    )
