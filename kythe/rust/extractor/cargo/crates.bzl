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
        name = "raze__adler__0_2_3",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/adler/adler-0.2.3.crate",
        type = "tar.gz",
        sha256 = "ee2a4ec343196209d6594e19543ae87a39f96d5534d7174822a3ad825dd6ed7e",
        strip_prefix = "adler-0.2.3",
        build_file = Label("//kythe/rust/extractor/cargo/remote:adler-0.2.3.BUILD"),
    )

    _new_http_archive(
        name = "raze__ansi_term__0_11_0",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/ansi_term/ansi_term-0.11.0.crate",
        type = "tar.gz",
        sha256 = "ee49baf6cb617b853aa8d93bf420db2383fab46d314482ca2803b40d5fde979b",
        strip_prefix = "ansi_term-0.11.0",
        build_file = Label("//kythe/rust/extractor/cargo/remote:ansi_term-0.11.0.BUILD"),
    )

    _new_http_archive(
        name = "raze__anyhow__1_0_31",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/anyhow/anyhow-1.0.31.crate",
        type = "tar.gz",
        sha256 = "85bb70cc08ec97ca5450e6eba421deeea5f172c0fc61f78b5357b2a8e8be195f",
        strip_prefix = "anyhow-1.0.31",
        build_file = Label("//kythe/rust/extractor/cargo/remote:anyhow-1.0.31.BUILD"),
    )

    _new_http_archive(
        name = "raze__atty__0_2_14",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/atty/atty-0.2.14.crate",
        type = "tar.gz",
        sha256 = "d9b39be18770d11421cdb1b9947a45dd3f37e93092cbf377614828a319d5fee8",
        strip_prefix = "atty-0.2.14",
        build_file = Label("//kythe/rust/extractor/cargo/remote:atty-0.2.14.BUILD"),
    )

    _new_http_archive(
        name = "raze__bitflags__1_2_1",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/bitflags/bitflags-1.2.1.crate",
        type = "tar.gz",
        sha256 = "cf1de2fe8c75bc145a2f577add951f8134889b4795d47466a54a5c846d691693",
        strip_prefix = "bitflags-1.2.1",
        build_file = Label("//kythe/rust/extractor/cargo/remote:bitflags-1.2.1.BUILD"),
    )

    _new_http_archive(
        name = "raze__bzip2__0_3_3",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/bzip2/bzip2-0.3.3.crate",
        type = "tar.gz",
        sha256 = "42b7c3cbf0fa9c1b82308d57191728ca0256cb821220f4e2fd410a72ade26e3b",
        strip_prefix = "bzip2-0.3.3",
        build_file = Label("//kythe/rust/extractor/cargo/remote:bzip2-0.3.3.BUILD"),
    )

    _new_http_archive(
        name = "raze__bzip2_sys__0_1_9_1_0_8",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/bzip2-sys/bzip2-sys-0.1.9+1.0.8.crate",
        type = "tar.gz",
        sha256 = "ad3b39a260062fca31f7b0b12f207e8f2590a67d32ec7d59c20484b07ea7285e",
        strip_prefix = "bzip2-sys-0.1.9+1.0.8",
        build_file = Label("//kythe/rust/extractor/cargo/remote:bzip2-sys-0.1.9+1.0.8.BUILD"),
    )

    _new_http_archive(
        name = "raze__cc__1_0_58",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/cc/cc-1.0.58.crate",
        type = "tar.gz",
        sha256 = "f9a06fb2e53271d7c279ec1efea6ab691c35a2ae67ec0d91d7acec0caf13b518",
        strip_prefix = "cc-1.0.58",
        build_file = Label("//kythe/rust/extractor/cargo/remote:cc-1.0.58.BUILD"),
    )

    _new_http_archive(
        name = "raze__cfg_if__0_1_10",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/cfg-if/cfg-if-0.1.10.crate",
        type = "tar.gz",
        sha256 = "4785bdd1c96b2a846b2bd7cc02e86b6b3dbf14e7e53446c4f54c92a361040822",
        strip_prefix = "cfg-if-0.1.10",
        build_file = Label("//kythe/rust/extractor/cargo/remote:cfg-if-0.1.10.BUILD"),
    )

    _new_http_archive(
        name = "raze__clap__2_33_1",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/clap/clap-2.33.1.crate",
        type = "tar.gz",
        sha256 = "bdfa80d47f954d53a35a64987ca1422f495b8d6483c0fe9f7117b36c2a792129",
        strip_prefix = "clap-2.33.1",
        build_file = Label("//kythe/rust/extractor/cargo/remote:clap-2.33.1.BUILD"),
    )

    _new_http_archive(
        name = "raze__crc32fast__1_2_0",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/crc32fast/crc32fast-1.2.0.crate",
        type = "tar.gz",
        sha256 = "ba125de2af0df55319f41944744ad91c71113bf74a4646efff39afe1f6842db1",
        strip_prefix = "crc32fast-1.2.0",
        build_file = Label("//kythe/rust/extractor/cargo/remote:crc32fast-1.2.0.BUILD"),
    )

    _new_http_archive(
        name = "raze__flate2__1_0_16",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/flate2/flate2-1.0.16.crate",
        type = "tar.gz",
        sha256 = "68c90b0fc46cf89d227cc78b40e494ff81287a92dd07631e5af0d06fe3cf885e",
        strip_prefix = "flate2-1.0.16",
        build_file = Label("//kythe/rust/extractor/cargo/remote:flate2-1.0.16.BUILD"),
    )

    _new_http_archive(
        name = "raze__fuchsia_cprng__0_1_1",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/fuchsia-cprng/fuchsia-cprng-0.1.1.crate",
        type = "tar.gz",
        sha256 = "a06f77d526c1a601b7c4cdd98f54b5eaabffc14d5f2f0296febdc7f357c6d3ba",
        strip_prefix = "fuchsia-cprng-0.1.1",
        build_file = Label("//kythe/rust/extractor/cargo/remote:fuchsia-cprng-0.1.1.BUILD"),
    )

    _new_http_archive(
        name = "raze__gcc__0_3_55",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/gcc/gcc-0.3.55.crate",
        type = "tar.gz",
        sha256 = "8f5f3913fa0bfe7ee1fd8248b6b9f42a5af4b9d65ec2dd2c3c26132b950ecfc2",
        strip_prefix = "gcc-0.3.55",
        build_file = Label("//kythe/rust/extractor/cargo/remote:gcc-0.3.55.BUILD"),
    )

    _new_http_archive(
        name = "raze__hermit_abi__0_1_15",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/hermit-abi/hermit-abi-0.1.15.crate",
        type = "tar.gz",
        sha256 = "3deed196b6e7f9e44a2ae8d94225d80302d81208b1bb673fd21fe634645c85a9",
        strip_prefix = "hermit-abi-0.1.15",
        build_file = Label("//kythe/rust/extractor/cargo/remote:hermit-abi-0.1.15.BUILD"),
    )

    _new_http_archive(
        name = "raze__libc__0_2_72",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/libc/libc-0.2.72.crate",
        type = "tar.gz",
        sha256 = "a9f8082297d534141b30c8d39e9b1773713ab50fdbe4ff30f750d063b3bfd701",
        strip_prefix = "libc-0.2.72",
        build_file = Label("//kythe/rust/extractor/cargo/remote:libc-0.2.72.BUILD"),
    )

    _new_http_archive(
        name = "raze__miniz_oxide__0_4_0",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/miniz_oxide/miniz_oxide-0.4.0.crate",
        type = "tar.gz",
        sha256 = "be0f75932c1f6cfae3c04000e40114adf955636e19040f9c0a2c380702aa1c7f",
        strip_prefix = "miniz_oxide-0.4.0",
        build_file = Label("//kythe/rust/extractor/cargo/remote:miniz_oxide-0.4.0.BUILD"),
    )

    _new_http_archive(
        name = "raze__pkg_config__0_3_18",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/pkg-config/pkg-config-0.3.18.crate",
        type = "tar.gz",
        sha256 = "d36492546b6af1463394d46f0c834346f31548646f6ba10849802c9c9a27ac33",
        strip_prefix = "pkg-config-0.3.18",
        build_file = Label("//kythe/rust/extractor/cargo/remote:pkg-config-0.3.18.BUILD"),
    )

    _new_http_archive(
        name = "raze__podio__0_1_7",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/podio/podio-0.1.7.crate",
        type = "tar.gz",
        sha256 = "b18befed8bc2b61abc79a457295e7e838417326da1586050b919414073977f19",
        strip_prefix = "podio-0.1.7",
        build_file = Label("//kythe/rust/extractor/cargo/remote:podio-0.1.7.BUILD"),
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
        build_file = Label("//kythe/rust/extractor/cargo/remote:protobuf-2.8.2.BUILD"),
    )

    _new_http_archive(
        name = "raze__quick_error__1_2_3",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/quick-error/quick-error-1.2.3.crate",
        type = "tar.gz",
        sha256 = "a1d01941d82fa2ab50be1e79e6714289dd7cde78eba4c074bc5a4374f650dfe0",
        strip_prefix = "quick-error-1.2.3",
        build_file = Label("//kythe/rust/extractor/cargo/remote:quick-error-1.2.3.BUILD"),
    )

    _new_http_archive(
        name = "raze__rand__0_3_23",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/rand/rand-0.3.23.crate",
        type = "tar.gz",
        sha256 = "64ac302d8f83c0c1974bf758f6b041c6c8ada916fbb44a609158ca8b064cc76c",
        strip_prefix = "rand-0.3.23",
        build_file = Label("//kythe/rust/extractor/cargo/remote:rand-0.3.23.BUILD"),
    )

    _new_http_archive(
        name = "raze__rand__0_4_6",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/rand/rand-0.4.6.crate",
        type = "tar.gz",
        sha256 = "552840b97013b1a26992c11eac34bdd778e464601a4c2054b5f0bff7c6761293",
        strip_prefix = "rand-0.4.6",
        build_file = Label("//kythe/rust/extractor/cargo/remote:rand-0.4.6.BUILD"),
    )

    _new_http_archive(
        name = "raze__rand_core__0_3_1",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/rand_core/rand_core-0.3.1.crate",
        type = "tar.gz",
        sha256 = "7a6fdeb83b075e8266dcc8762c22776f6877a63111121f5f8c7411e5be7eed4b",
        strip_prefix = "rand_core-0.3.1",
        build_file = Label("//kythe/rust/extractor/cargo/remote:rand_core-0.3.1.BUILD"),
    )

    _new_http_archive(
        name = "raze__rand_core__0_4_2",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/rand_core/rand_core-0.4.2.crate",
        type = "tar.gz",
        sha256 = "9c33a3c44ca05fa6f1807d8e6743f3824e8509beca625669633be0acbdf509dc",
        strip_prefix = "rand_core-0.4.2",
        build_file = Label("//kythe/rust/extractor/cargo/remote:rand_core-0.4.2.BUILD"),
    )

    _new_http_archive(
        name = "raze__rdrand__0_4_0",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/rdrand/rdrand-0.4.0.crate",
        type = "tar.gz",
        sha256 = "678054eb77286b51581ba43620cc911abf02758c91f93f479767aed0f90458b2",
        strip_prefix = "rdrand-0.4.0",
        build_file = Label("//kythe/rust/extractor/cargo/remote:rdrand-0.4.0.BUILD"),
    )

    _new_http_archive(
        name = "raze__remove_dir_all__0_5_3",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/remove_dir_all/remove_dir_all-0.5.3.crate",
        type = "tar.gz",
        sha256 = "3acd125665422973a33ac9d3dd2df85edad0f4ae9b00dafb1a05e43a9f5ef8e7",
        strip_prefix = "remove_dir_all-0.5.3",
        build_file = Label("//kythe/rust/extractor/cargo/remote:remove_dir_all-0.5.3.BUILD"),
    )

    _new_http_archive(
        name = "raze__rls_data__0_19_0",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/rls-data/rls-data-0.19.0.crate",
        type = "tar.gz",
        sha256 = "76c72ea97e045be5f6290bb157ebdc5ee9f2b093831ff72adfaf59025cf5c491",
        strip_prefix = "rls-data-0.19.0",
        build_file = Label("//kythe/rust/extractor/cargo/remote:rls-data-0.19.0.BUILD"),
    )

    _new_http_archive(
        name = "raze__rls_span__0_5_2",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/rls-span/rls-span-0.5.2.crate",
        type = "tar.gz",
        sha256 = "f2e9bed56f6272bd85d9d06d1aaeef80c5fddc78a82199eb36dceb5f94e7d934",
        strip_prefix = "rls-span-0.5.2",
        build_file = Label("//kythe/rust/extractor/cargo/remote:rls-span-0.5.2.BUILD"),
    )

    _new_http_archive(
        name = "raze__rust_crypto__0_2_36",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/rust-crypto/rust-crypto-0.2.36.crate",
        type = "tar.gz",
        sha256 = "f76d05d3993fd5f4af9434e8e436db163a12a9d40e1a58a726f27a01dfd12a2a",
        strip_prefix = "rust-crypto-0.2.36",
        build_file = Label("//kythe/rust/extractor/cargo/remote:rust-crypto-0.2.36.BUILD"),
    )

    _new_http_archive(
        name = "raze__rustc_serialize__0_3_24",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/rustc-serialize/rustc-serialize-0.3.24.crate",
        type = "tar.gz",
        sha256 = "dcf128d1287d2ea9d80910b5f1120d0b8eede3fbf1abe91c40d39ea7d51e6fda",
        strip_prefix = "rustc-serialize-0.3.24",
        build_file = Label("//kythe/rust/extractor/cargo/remote:rustc-serialize-0.3.24.BUILD"),
    )

    _new_http_archive(
        name = "raze__serde__1_0_114",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/serde/serde-1.0.114.crate",
        type = "tar.gz",
        sha256 = "5317f7588f0a5078ee60ef675ef96735a1442132dc645eb1d12c018620ed8cd3",
        strip_prefix = "serde-1.0.114",
        build_file = Label("//kythe/rust/extractor/cargo/remote:serde-1.0.114.BUILD"),
    )

    _new_http_archive(
        name = "raze__strsim__0_8_0",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/strsim/strsim-0.8.0.crate",
        type = "tar.gz",
        sha256 = "8ea5119cdb4c55b55d432abb513a0429384878c15dde60cc77b1c99de1a95a6a",
        strip_prefix = "strsim-0.8.0",
        build_file = Label("//kythe/rust/extractor/cargo/remote:strsim-0.8.0.BUILD"),
    )

    _new_http_archive(
        name = "raze__tempdir__0_3_7",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/tempdir/tempdir-0.3.7.crate",
        type = "tar.gz",
        sha256 = "15f2b5fb00ccdf689e0149d1b1b3c03fead81c2b37735d812fa8bddbbf41b6d8",
        strip_prefix = "tempdir-0.3.7",
        build_file = Label("//kythe/rust/extractor/cargo/remote:tempdir-0.3.7.BUILD"),
    )

    _new_http_archive(
        name = "raze__textwrap__0_11_0",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/textwrap/textwrap-0.11.0.crate",
        type = "tar.gz",
        sha256 = "d326610f408c7a4eb6f51c37c330e496b08506c9457c9d34287ecc38809fb060",
        strip_prefix = "textwrap-0.11.0",
        build_file = Label("//kythe/rust/extractor/cargo/remote:textwrap-0.11.0.BUILD"),
    )

    _new_http_archive(
        name = "raze__time__0_1_43",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/time/time-0.1.43.crate",
        type = "tar.gz",
        sha256 = "ca8a50ef2360fbd1eeb0ecd46795a87a19024eb4b53c5dc916ca1fd95fe62438",
        strip_prefix = "time-0.1.43",
        build_file = Label("//kythe/rust/extractor/cargo/remote:time-0.1.43.BUILD"),
    )

    _new_http_archive(
        name = "raze__unicode_width__0_1_8",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/unicode-width/unicode-width-0.1.8.crate",
        type = "tar.gz",
        sha256 = "9337591893a19b88d8d87f2cec1e73fad5cdfd10e5a6f349f498ad6ea2ffb1e3",
        strip_prefix = "unicode-width-0.1.8",
        build_file = Label("//kythe/rust/extractor/cargo/remote:unicode-width-0.1.8.BUILD"),
    )

    _new_http_archive(
        name = "raze__vec_map__0_8_2",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/vec_map/vec_map-0.8.2.crate",
        type = "tar.gz",
        sha256 = "f1bddf1187be692e79c5ffeab891132dfb0f236ed36a43c7ed39f1165ee20191",
        strip_prefix = "vec_map-0.8.2",
        build_file = Label("//kythe/rust/extractor/cargo/remote:vec_map-0.8.2.BUILD"),
    )

    _new_http_archive(
        name = "raze__winapi__0_3_9",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/winapi/winapi-0.3.9.crate",
        type = "tar.gz",
        sha256 = "5c839a674fcd7a98952e593242ea400abe93992746761e38641405d28b00f419",
        strip_prefix = "winapi-0.3.9",
        build_file = Label("//kythe/rust/extractor/cargo/remote:winapi-0.3.9.BUILD"),
    )

    _new_http_archive(
        name = "raze__winapi_i686_pc_windows_gnu__0_4_0",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/winapi-i686-pc-windows-gnu/winapi-i686-pc-windows-gnu-0.4.0.crate",
        type = "tar.gz",
        sha256 = "ac3b87c63620426dd9b991e5ce0329eff545bccbbb34f3be09ff6fb6ab51b7b6",
        strip_prefix = "winapi-i686-pc-windows-gnu-0.4.0",
        build_file = Label("//kythe/rust/extractor/cargo/remote:winapi-i686-pc-windows-gnu-0.4.0.BUILD"),
    )

    _new_http_archive(
        name = "raze__winapi_x86_64_pc_windows_gnu__0_4_0",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/winapi-x86_64-pc-windows-gnu/winapi-x86_64-pc-windows-gnu-0.4.0.crate",
        type = "tar.gz",
        sha256 = "712e227841d057c1ee1cd2fb22fa7e5a5461ae8e48fa2ca79ec42cfc1931183f",
        strip_prefix = "winapi-x86_64-pc-windows-gnu-0.4.0",
        build_file = Label("//kythe/rust/extractor/cargo/remote:winapi-x86_64-pc-windows-gnu-0.4.0.BUILD"),
    )

    _new_http_archive(
        name = "raze__zip__0_5_6",
        url = "https://crates-io.s3-us-west-1.amazonaws.com/crates/zip/zip-0.5.6.crate",
        type = "tar.gz",
        sha256 = "58287c28d78507f5f91f2a4cf1e8310e2c76fd4c6932f93ac60fd1ceb402db7d",
        strip_prefix = "zip-0.5.6",
        build_file = Label("//kythe/rust/extractor/cargo/remote:zip-0.5.6.BUILD"),
    )

