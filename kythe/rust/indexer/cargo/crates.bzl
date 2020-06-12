"""
cargo-raze crate workspace functions

DO NOT EDIT! Replaced on runs of cargo-raze
"""
load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")
load("@bazel_tools//tools/build_defs/repo:git.bzl", "new_git_repository")

def _new_http_archive(name, **kwargs):
    if not native.existing_rule(name):
        http_archive(name=name, **kwargs)

def _new_git_repository(name, **kwargs):
    if not native.existing_rule(name):
        new_git_repository(name=name, **kwargs)

def raze_fetch_remote_crates():

    _new_http_archive(
        name = "raze__adler32__1_0_4",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/adler32/adler32-1.0.4.crate",
        type = "tar.gz",
        sha256 = "5d2e7343e7fc9de883d1b0341e0b13970f764c14101234857d2ddafa1cb1cac2",
        strip_prefix = "adler32-1.0.4",
        build_file = Label("//kythe/rust/indexer/cargo/remote:adler32-1.0.4.BUILD"),
    )

    _new_http_archive(
        name = "raze__arrayvec__0_4_12",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/arrayvec/arrayvec-0.4.12.crate",
        type = "tar.gz",
        sha256 = "cd9fd44efafa8690358b7408d253adf110036b88f55672a933f01d616ad9b1b9",
        strip_prefix = "arrayvec-0.4.12",
        build_file = Label("//kythe/rust/indexer/cargo/remote:arrayvec-0.4.12.BUILD"),
    )

    _new_http_archive(
        name = "raze__autocfg__1_0_0",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/autocfg/autocfg-1.0.0.crate",
        type = "tar.gz",
        sha256 = "f8aac770f1885fd7e387acedd76065302551364496e46b3dd00860b2f8359b9d",
        strip_prefix = "autocfg-1.0.0",
        build_file = Label("//kythe/rust/indexer/cargo/remote:autocfg-1.0.0.BUILD"),
    )

    _new_http_archive(
        name = "raze__bitflags__1_2_1",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/bitflags/bitflags-1.2.1.crate",
        type = "tar.gz",
        sha256 = "cf1de2fe8c75bc145a2f577add951f8134889b4795d47466a54a5c846d691693",
        strip_prefix = "bitflags-1.2.1",
        build_file = Label("//kythe/rust/indexer/cargo/remote:bitflags-1.2.1.BUILD"),
    )

    _new_http_archive(
        name = "raze__bstr__0_2_13",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/bstr/bstr-0.2.13.crate",
        type = "tar.gz",
        sha256 = "31accafdb70df7871592c058eca3985b71104e15ac32f64706022c58867da931",
        strip_prefix = "bstr-0.2.13",
        build_file = Label("//kythe/rust/indexer/cargo/remote:bstr-0.2.13.BUILD"),
    )

    _new_http_archive(
        name = "raze__byteorder__0_5_3",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/byteorder/byteorder-0.5.3.crate",
        type = "tar.gz",
        sha256 = "0fc10e8cc6b2580fda3f36eb6dc5316657f812a3df879a44a66fc9f0fdbc4855",
        strip_prefix = "byteorder-0.5.3",
        build_file = Label("//kythe/rust/indexer/cargo/remote:byteorder-0.5.3.BUILD"),
    )

    _new_http_archive(
        name = "raze__byteorder__1_3_4",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/byteorder/byteorder-1.3.4.crate",
        type = "tar.gz",
        sha256 = "08c48aae112d48ed9f069b33538ea9e3e90aa263cfa3d1c24309612b1f7472de",
        strip_prefix = "byteorder-1.3.4",
        build_file = Label("//kythe/rust/indexer/cargo/remote:byteorder-1.3.4.BUILD"),
    )

    _new_http_archive(
        name = "raze__cfg_if__0_1_9",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/cfg-if/cfg-if-0.1.9.crate",
        type = "tar.gz",
        sha256 = "b486ce3ccf7ffd79fdeb678eac06a9e6c09fc88d33836340becb8fffe87c5e33",
        strip_prefix = "cfg-if-0.1.9",
        build_file = Label("//kythe/rust/indexer/cargo/remote:cfg-if-0.1.9.BUILD"),
    )

    _new_http_archive(
        name = "raze__chardet__0_2_4",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/chardet/chardet-0.2.4.crate",
        type = "tar.gz",
        sha256 = "1a48563284b67c003ba0fb7243c87fab68885e1532c605704228a80238512e31",
        strip_prefix = "chardet-0.2.4",
        build_file = Label("//kythe/rust/indexer/cargo/remote:chardet-0.2.4.BUILD"),
    )

    _new_http_archive(
        name = "raze__chrono__0_4_11",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/chrono/chrono-0.4.11.crate",
        type = "tar.gz",
        sha256 = "80094f509cf8b5ae86a4966a39b3ff66cd7e2a3e594accec3743ff3fabeab5b2",
        strip_prefix = "chrono-0.4.11",
        build_file = Label("//kythe/rust/indexer/cargo/remote:chrono-0.4.11.BUILD"),
    )

    _new_http_archive(
        name = "raze__circular__0_3_0",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/circular/circular-0.3.0.crate",
        type = "tar.gz",
        sha256 = "b0fc239e0f6cb375d2402d48afb92f76f5404fd1df208a41930ec81eda078bea",
        strip_prefix = "circular-0.3.0",
        build_file = Label("//kythe/rust/indexer/cargo/remote:circular-0.3.0.BUILD"),
    )

    _new_http_archive(
        name = "raze__codepage_437__0_1_0",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/codepage-437/codepage-437-0.1.0.crate",
        type = "tar.gz",
        sha256 = "e40c1169585d8d08e5675a39f2fc056cd19a258fc4cba5e3bbf4a9c1026de535",
        strip_prefix = "codepage-437-0.1.0",
        build_file = Label("//kythe/rust/indexer/cargo/remote:codepage-437-0.1.0.BUILD"),
    )

    _new_http_archive(
        name = "raze__crc32fast__1_2_0",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/crc32fast/crc32fast-1.2.0.crate",
        type = "tar.gz",
        sha256 = "ba125de2af0df55319f41944744ad91c71113bf74a4646efff39afe1f6842db1",
        strip_prefix = "crc32fast-1.2.0",
        build_file = Label("//kythe/rust/indexer/cargo/remote:crc32fast-1.2.0.BUILD"),
    )

    _new_http_archive(
        name = "raze__csv__1_1_3",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/csv/csv-1.1.3.crate",
        type = "tar.gz",
        sha256 = "00affe7f6ab566df61b4be3ce8cf16bc2576bca0963ceb0955e45d514bf9a279",
        strip_prefix = "csv-1.1.3",
        build_file = Label("//kythe/rust/indexer/cargo/remote:csv-1.1.3.BUILD"),
    )

    _new_http_archive(
        name = "raze__csv_core__0_1_10",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/csv-core/csv-core-0.1.10.crate",
        type = "tar.gz",
        sha256 = "2b2466559f260f48ad25fe6317b3c8dac77b5bdb5763ac7d9d6103530663bc90",
        strip_prefix = "csv-core-0.1.10",
        build_file = Label("//kythe/rust/indexer/cargo/remote:csv-core-0.1.10.BUILD"),
    )

    _new_http_archive(
        name = "raze__encoding_rs__0_8_23",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/encoding_rs/encoding_rs-0.8.23.crate",
        type = "tar.gz",
        sha256 = "e8ac63f94732332f44fe654443c46f6375d1939684c17b0afb6cb56b0456e171",
        strip_prefix = "encoding_rs-0.8.23",
        build_file = Label("//kythe/rust/indexer/cargo/remote:encoding_rs-0.8.23.BUILD"),
    )

    _new_http_archive(
        name = "raze__hex_fmt__0_3_0",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/hex_fmt/hex_fmt-0.3.0.crate",
        type = "tar.gz",
        sha256 = "b07f60793ff0a4d9cef0f18e63b5357e06209987153a64648c972c1e5aff336f",
        strip_prefix = "hex_fmt-0.3.0",
        build_file = Label("//kythe/rust/indexer/cargo/remote:hex_fmt-0.3.0.BUILD"),
    )

    _new_http_archive(
        name = "raze__itoa__0_4_5",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/itoa/itoa-0.4.5.crate",
        type = "tar.gz",
        sha256 = "b8b7a7c0c47db5545ed3fef7468ee7bb5b74691498139e4b3f6a20685dc6dd8e",
        strip_prefix = "itoa-0.4.5",
        build_file = Label("//kythe/rust/indexer/cargo/remote:itoa-0.4.5.BUILD"),
    )

    _new_http_archive(
        name = "raze__kernel32_sys__0_2_2",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/kernel32-sys/kernel32-sys-0.2.2.crate",
        type = "tar.gz",
        sha256 = "7507624b29483431c0ba2d82aece8ca6cdba9382bff4ddd0f7490560c056098d",
        strip_prefix = "kernel32-sys-0.2.2",
        build_file = Label("//kythe/rust/indexer/cargo/remote:kernel32-sys-0.2.2.BUILD"),
    )

    _new_http_archive(
        name = "raze__lazy_static__1_4_0",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/lazy_static/lazy_static-1.4.0.crate",
        type = "tar.gz",
        sha256 = "e2abad23fbc42b3700f2f279844dc832adb2b2eb069b2df918f455c4e18cc646",
        strip_prefix = "lazy_static-1.4.0",
        build_file = Label("//kythe/rust/indexer/cargo/remote:lazy_static-1.4.0.BUILD"),
    )

    _new_http_archive(
        name = "raze__lexical_core__0_6_7",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/lexical-core/lexical-core-0.6.7.crate",
        type = "tar.gz",
        sha256 = "f86d66d380c9c5a685aaac7a11818bdfa1f733198dfd9ec09c70b762cd12ad6f",
        strip_prefix = "lexical-core-0.6.7",
        build_file = Label("//kythe/rust/indexer/cargo/remote:lexical-core-0.6.7.BUILD"),
    )

    _new_http_archive(
        name = "raze__libc__0_2_71",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/libc/libc-0.2.71.crate",
        type = "tar.gz",
        sha256 = "9457b06509d27052635f90d6466700c65095fdf75409b3fbdd903e988b886f49",
        strip_prefix = "libc-0.2.71",
        build_file = Label("//kythe/rust/indexer/cargo/remote:libc-0.2.71.BUILD"),
    )

    _new_http_archive(
        name = "raze__libflate__0_1_27",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/libflate/libflate-0.1.27.crate",
        type = "tar.gz",
        sha256 = "d9135df43b1f5d0e333385cb6e7897ecd1a43d7d11b91ac003f4d2c2d2401fdd",
        strip_prefix = "libflate-0.1.27",
        build_file = Label("//kythe/rust/indexer/cargo/remote:libflate-0.1.27.BUILD"),
    )

    _new_http_archive(
        name = "raze__log__0_4_8",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/log/log-0.4.8.crate",
        type = "tar.gz",
        sha256 = "14b6052be84e6b71ab17edffc2eeabf5c2c3ae1fdb464aae35ac50c67a44e1f7",
        strip_prefix = "log-0.4.8",
        build_file = Label("//kythe/rust/indexer/cargo/remote:log-0.4.8.BUILD"),
    )

    _new_http_archive(
        name = "raze__memchr__2_3_3",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/memchr/memchr-2.3.3.crate",
        type = "tar.gz",
        sha256 = "3728d817d99e5ac407411fa471ff9800a778d88a24685968b36824eaf4bee400",
        strip_prefix = "memchr-2.3.3",
        build_file = Label("//kythe/rust/indexer/cargo/remote:memchr-2.3.3.BUILD"),
    )

    _new_http_archive(
        name = "raze__nodrop__0_1_14",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/nodrop/nodrop-0.1.14.crate",
        type = "tar.gz",
        sha256 = "72ef4a56884ca558e5ddb05a1d1e7e1bfd9a68d9ed024c21704cc98872dae1bb",
        strip_prefix = "nodrop-0.1.14",
        build_file = Label("//kythe/rust/indexer/cargo/remote:nodrop-0.1.14.BUILD"),
    )

    _new_http_archive(
        name = "raze__nom__5_1_1",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/nom/nom-5.1.1.crate",
        type = "tar.gz",
        sha256 = "0b471253da97532da4b61552249c521e01e736071f71c1a4f7ebbfbf0a06aad6",
        strip_prefix = "nom-5.1.1",
        build_file = Label("//kythe/rust/indexer/cargo/remote:nom-5.1.1.BUILD"),
    )

    _new_http_archive(
        name = "raze__num_integer__0_1_42",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/num-integer/num-integer-0.1.42.crate",
        type = "tar.gz",
        sha256 = "3f6ea62e9d81a77cd3ee9a2a5b9b609447857f3d358704331e4ef39eb247fcba",
        strip_prefix = "num-integer-0.1.42",
        build_file = Label("//kythe/rust/indexer/cargo/remote:num-integer-0.1.42.BUILD"),
    )

    _new_http_archive(
        name = "raze__num_traits__0_2_11",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/num-traits/num-traits-0.2.11.crate",
        type = "tar.gz",
        sha256 = "c62be47e61d1842b9170f0fdeec8eba98e60e90e5446449a0545e5152acd7096",
        strip_prefix = "num-traits-0.2.11",
        build_file = Label("//kythe/rust/indexer/cargo/remote:num-traits-0.2.11.BUILD"),
    )

    _new_http_archive(
        name = "raze__positioned_io__0_2_2",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/positioned-io/positioned-io-0.2.2.crate",
        type = "tar.gz",
        sha256 = "c405a565f48a728dbb07fa1770e30791b0fa3e6344c1e5615225ce84049354d6",
        strip_prefix = "positioned-io-0.2.2",
        build_file = Label("//kythe/rust/indexer/cargo/remote:positioned-io-0.2.2.BUILD"),
    )

    _new_http_archive(
        name = "raze__pretty_hex__0_1_1",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/pretty-hex/pretty-hex-0.1.1.crate",
        type = "tar.gz",
        sha256 = "be91bcc43e73799dc46a6c194a55e7aae1d86cc867c860fd4a436019af21bd8c",
        strip_prefix = "pretty-hex-0.1.1",
        build_file = Label("//kythe/rust/indexer/cargo/remote:pretty-hex-0.1.1.BUILD"),
    )

    _new_http_archive(
        name = "raze__protobuf__2_8_2",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/protobuf/protobuf-2.8.2.crate",
        type = "tar.gz",
        sha256 = "70731852eec72c56d11226c8a5f96ad5058a3dab73647ca5f7ee351e464f2571",
        strip_prefix = "protobuf-2.8.2",
        patches = [
            "@io_bazel_rules_rust//proto/raze/patch:protobuf-2.8.2.patch",
        ],
        patch_args = [
            "-p1",
        ],
        build_file = Label("//kythe/rust/indexer/cargo/remote:protobuf-2.8.2.BUILD"),
    )

    _new_http_archive(
        name = "raze__quick_error__1_2_3",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/quick-error/quick-error-1.2.3.crate",
        type = "tar.gz",
        sha256 = "a1d01941d82fa2ab50be1e79e6714289dd7cde78eba4c074bc5a4374f650dfe0",
        strip_prefix = "quick-error-1.2.3",
        build_file = Label("//kythe/rust/indexer/cargo/remote:quick-error-1.2.3.BUILD"),
    )

    _new_http_archive(
        name = "raze__rc_zip__0_0_1",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/rc-zip/rc-zip-0.0.1.crate",
        type = "tar.gz",
        sha256 = "687c7b93ace7e55680577fe4a2f552cf2c631b65ed96206dadeb70398c34cba0",
        strip_prefix = "rc-zip-0.0.1",
        build_file = Label("//kythe/rust/indexer/cargo/remote:rc-zip-0.0.1.BUILD"),
    )

    _new_http_archive(
        name = "raze__regex_automata__0_1_9",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/regex-automata/regex-automata-0.1.9.crate",
        type = "tar.gz",
        sha256 = "ae1ded71d66a4a97f5e961fd0cb25a5f366a42a41570d16a763a69c092c26ae4",
        strip_prefix = "regex-automata-0.1.9",
        build_file = Label("//kythe/rust/indexer/cargo/remote:regex-automata-0.1.9.BUILD"),
    )

    _new_http_archive(
        name = "raze__rle_decode_fast__1_0_1",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/rle-decode-fast/rle-decode-fast-1.0.1.crate",
        type = "tar.gz",
        sha256 = "cabe4fa914dec5870285fa7f71f602645da47c486e68486d2b4ceb4a343e90ac",
        strip_prefix = "rle-decode-fast-1.0.1",
        build_file = Label("//kythe/rust/indexer/cargo/remote:rle-decode-fast-1.0.1.BUILD"),
    )

    _new_http_archive(
        name = "raze__rustc_version__0_2_3",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/rustc_version/rustc_version-0.2.3.crate",
        type = "tar.gz",
        sha256 = "138e3e0acb6c9fb258b19b67cb8abd63c00679d2851805ea151465464fe9030a",
        strip_prefix = "rustc_version-0.2.3",
        build_file = Label("//kythe/rust/indexer/cargo/remote:rustc_version-0.2.3.BUILD"),
    )

    _new_http_archive(
        name = "raze__ryu__1_0_5",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/ryu/ryu-1.0.5.crate",
        type = "tar.gz",
        sha256 = "71d301d4193d031abdd79ff7e3dd721168a9572ef3fe51a1517aba235bd8f86e",
        strip_prefix = "ryu-1.0.5",
        build_file = Label("//kythe/rust/indexer/cargo/remote:ryu-1.0.5.BUILD"),
    )

    _new_http_archive(
        name = "raze__semver__0_9_0",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/semver/semver-0.9.0.crate",
        type = "tar.gz",
        sha256 = "1d7eb9ef2c18661902cc47e535f9bc51b78acd254da71d375c2f6720d9a40403",
        strip_prefix = "semver-0.9.0",
        build_file = Label("//kythe/rust/indexer/cargo/remote:semver-0.9.0.BUILD"),
    )

    _new_http_archive(
        name = "raze__semver_parser__0_7_0",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/semver-parser/semver-parser-0.7.0.crate",
        type = "tar.gz",
        sha256 = "388a1df253eca08550bef6c72392cfe7c30914bf41df5269b68cbd6ff8f570a3",
        strip_prefix = "semver-parser-0.7.0",
        build_file = Label("//kythe/rust/indexer/cargo/remote:semver-parser-0.7.0.BUILD"),
    )

    _new_http_archive(
        name = "raze__serde__1_0_111",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/serde/serde-1.0.111.crate",
        type = "tar.gz",
        sha256 = "c9124df5b40cbd380080b2cc6ab894c040a3070d995f5c9dc77e18c34a8ae37d",
        strip_prefix = "serde-1.0.111",
        build_file = Label("//kythe/rust/indexer/cargo/remote:serde-1.0.111.BUILD"),
    )

    _new_http_archive(
        name = "raze__static_assertions__0_3_4",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/static_assertions/static_assertions-0.3.4.crate",
        type = "tar.gz",
        sha256 = "7f3eb36b47e512f8f1c9e3d10c2c1965bc992bd9cdb024fa581e2194501c83d3",
        strip_prefix = "static_assertions-0.3.4",
        build_file = Label("//kythe/rust/indexer/cargo/remote:static_assertions-0.3.4.BUILD"),
    )

    _new_http_archive(
        name = "raze__take_mut__0_2_2",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/take_mut/take_mut-0.2.2.crate",
        type = "tar.gz",
        sha256 = "f764005d11ee5f36500a149ace24e00e3da98b0158b3e2d53a7495660d3f4d60",
        strip_prefix = "take_mut-0.2.2",
        build_file = Label("//kythe/rust/indexer/cargo/remote:take_mut-0.2.2.BUILD"),
    )

    _new_http_archive(
        name = "raze__time__0_1_43",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/time/time-0.1.43.crate",
        type = "tar.gz",
        sha256 = "ca8a50ef2360fbd1eeb0ecd46795a87a19024eb4b53c5dc916ca1fd95fe62438",
        strip_prefix = "time-0.1.43",
        build_file = Label("//kythe/rust/indexer/cargo/remote:time-0.1.43.BUILD"),
    )

    _new_http_archive(
        name = "raze__version_check__0_9_2",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/version_check/version_check-0.9.2.crate",
        type = "tar.gz",
        sha256 = "b5a972e5669d67ba988ce3dc826706fb0a8b01471c088cb0b6110b805cc36aed",
        strip_prefix = "version_check-0.9.2",
        build_file = Label("//kythe/rust/indexer/cargo/remote:version_check-0.9.2.BUILD"),
    )

    _new_http_archive(
        name = "raze__winapi__0_2_8",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/winapi/winapi-0.2.8.crate",
        type = "tar.gz",
        sha256 = "167dc9d6949a9b857f3451275e911c3f44255842c1f7a76f33c55103a909087a",
        strip_prefix = "winapi-0.2.8",
        build_file = Label("//kythe/rust/indexer/cargo/remote:winapi-0.2.8.BUILD"),
    )

    _new_http_archive(
        name = "raze__winapi__0_3_8",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/winapi/winapi-0.3.8.crate",
        type = "tar.gz",
        sha256 = "8093091eeb260906a183e6ae1abdba2ef5ef2257a21801128899c3fc699229c6",
        strip_prefix = "winapi-0.3.8",
        build_file = Label("//kythe/rust/indexer/cargo/remote:winapi-0.3.8.BUILD"),
    )

    _new_http_archive(
        name = "raze__winapi_build__0_1_1",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/winapi-build/winapi-build-0.1.1.crate",
        type = "tar.gz",
        sha256 = "2d315eee3b34aca4797b2da6b13ed88266e6d612562a0c46390af8299fc699bc",
        strip_prefix = "winapi-build-0.1.1",
        build_file = Label("//kythe/rust/indexer/cargo/remote:winapi-build-0.1.1.BUILD"),
    )

    _new_http_archive(
        name = "raze__winapi_i686_pc_windows_gnu__0_4_0",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/winapi-i686-pc-windows-gnu/winapi-i686-pc-windows-gnu-0.4.0.crate",
        type = "tar.gz",
        sha256 = "ac3b87c63620426dd9b991e5ce0329eff545bccbbb34f3be09ff6fb6ab51b7b6",
        strip_prefix = "winapi-i686-pc-windows-gnu-0.4.0",
        build_file = Label("//kythe/rust/indexer/cargo/remote:winapi-i686-pc-windows-gnu-0.4.0.BUILD"),
    )

    _new_http_archive(
        name = "raze__winapi_x86_64_pc_windows_gnu__0_4_0",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/winapi-x86_64-pc-windows-gnu/winapi-x86_64-pc-windows-gnu-0.4.0.crate",
        type = "tar.gz",
        sha256 = "712e227841d057c1ee1cd2fb22fa7e5a5461ae8e48fa2ca79ec42cfc1931183f",
        strip_prefix = "winapi-x86_64-pc-windows-gnu-0.4.0",
        build_file = Label("//kythe/rust/indexer/cargo/remote:winapi-x86_64-pc-windows-gnu-0.4.0.BUILD"),
    )

