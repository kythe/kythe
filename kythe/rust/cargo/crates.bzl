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
        name = "raze__ansi_term__0_12_1",
        url = "https://crates.io/api/v1/crates/ansi_term/0.12.1/download",
        type = "tar.gz",
        sha256 = "d52a9bb7ec0cf484c551830a7ce27bd20d67eac647e1befb56b0be4ee39a55d2",
        strip_prefix = "ansi_term-0.12.1",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.ansi_term-0.12.1.bazel"),
    )

    maybe(
        http_archive,
        name = "raze__anyhow__1_0_57",
        url = "https://crates.io/api/v1/crates/anyhow/1.0.57/download",
        type = "tar.gz",
        sha256 = "08f9b8508dccb7687a1d6c4ce66b2b0ecef467c94667de27d8d7fe1f8d2a9cdc",
        strip_prefix = "anyhow-1.0.57",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.anyhow-1.0.57.bazel"),
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
        name = "raze__autocfg__1_1_0",
        url = "https://crates.io/api/v1/crates/autocfg/1.1.0/download",
        type = "tar.gz",
        sha256 = "d468802bab17cbc0cc575e9b053f41e72aa36bfa6b7f55e3529ffa43161b97fa",
        strip_prefix = "autocfg-1.1.0",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.autocfg-1.1.0.bazel"),
    )

    maybe(
        http_archive,
        name = "raze__base64__0_13_0",
        url = "https://crates.io/api/v1/crates/base64/0.13.0/download",
        type = "tar.gz",
        sha256 = "904dfeac50f3cdaba28fc6f57fdcddb75f49ed61346676a78c4ffe55877802fd",
        strip_prefix = "base64-0.13.0",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.base64-0.13.0.bazel"),
    )

    maybe(
        http_archive,
        name = "raze__bitflags__1_3_2",
        url = "https://crates.io/api/v1/crates/bitflags/1.3.2/download",
        type = "tar.gz",
        sha256 = "bef38d45163c2f1dde094a7dfd33ccf595c92905c8f8f4fdc18d06fb1037718a",
        strip_prefix = "bitflags-1.3.2",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.bitflags-1.3.2.bazel"),
    )

    maybe(
        http_archive,
        name = "raze__block_buffer__0_9_0",
        url = "https://crates.io/api/v1/crates/block-buffer/0.9.0/download",
        type = "tar.gz",
        sha256 = "4152116fd6e9dadb291ae18fc1ec3575ed6d84c29642d97890f4b4a3417297e4",
        strip_prefix = "block-buffer-0.9.0",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.block-buffer-0.9.0.bazel"),
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
        name = "raze__bzip2__0_4_3",
        url = "https://crates.io/api/v1/crates/bzip2/0.4.3/download",
        type = "tar.gz",
        sha256 = "6afcd980b5f3a45017c57e57a2fcccbb351cc43a356ce117ef760ef8052b89b0",
        strip_prefix = "bzip2-0.4.3",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.bzip2-0.4.3.bazel"),
    )

    maybe(
        http_archive,
        name = "raze__bzip2_sys__0_1_11_1_0_8",
        url = "https://crates.io/api/v1/crates/bzip2-sys/0.1.11+1.0.8/download",
        type = "tar.gz",
        sha256 = "736a955f3fa7875102d57c82b8cac37ec45224a07fd32d58f9f7a186b6cd4cdc",
        strip_prefix = "bzip2-sys-0.1.11+1.0.8",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.bzip2-sys-0.1.11+1.0.8.bazel"),
    )

    maybe(
        http_archive,
        name = "raze__cc__1_0_73",
        url = "https://crates.io/api/v1/crates/cc/1.0.73/download",
        type = "tar.gz",
        sha256 = "2fff2a6927b3bb87f9595d67196a70493f627687a71d87a0d692242c33f58c11",
        strip_prefix = "cc-1.0.73",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.cc-1.0.73.bazel"),
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
        name = "raze__clap__2_34_0",
        url = "https://crates.io/api/v1/crates/clap/2.34.0/download",
        type = "tar.gz",
        sha256 = "a0610544180c38b88101fecf2dd634b174a62eef6946f84dfc6a7127512b381c",
        strip_prefix = "clap-2.34.0",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.clap-2.34.0.bazel"),
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
        name = "raze__cpufeatures__0_2_2",
        url = "https://crates.io/api/v1/crates/cpufeatures/0.2.2/download",
        type = "tar.gz",
        sha256 = "59a6001667ab124aebae2a495118e11d30984c3a653e99d86d58971708cf5e4b",
        strip_prefix = "cpufeatures-0.2.2",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.cpufeatures-0.2.2.bazel"),
    )

    maybe(
        http_archive,
        name = "raze__crc32fast__1_3_2",
        url = "https://crates.io/api/v1/crates/crc32fast/1.3.2/download",
        type = "tar.gz",
        sha256 = "b540bd8bc810d3885c6ea91e2018302f68baba2129ab3e88f32389ee9370880d",
        strip_prefix = "crc32fast-1.3.2",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.crc32fast-1.3.2.bazel"),
    )

    maybe(
        http_archive,
        name = "raze__crossbeam_channel__0_5_4",
        url = "https://crates.io/api/v1/crates/crossbeam-channel/0.5.4/download",
        type = "tar.gz",
        sha256 = "5aaa7bd5fb665c6864b5f963dd9097905c54125909c7aa94c9e18507cdbe6c53",
        strip_prefix = "crossbeam-channel-0.5.4",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.crossbeam-channel-0.5.4.bazel"),
    )

    maybe(
        http_archive,
        name = "raze__crossbeam_deque__0_8_1",
        url = "https://crates.io/api/v1/crates/crossbeam-deque/0.8.1/download",
        type = "tar.gz",
        sha256 = "6455c0ca19f0d2fbf751b908d5c55c1f5cbc65e03c4225427254b46890bdde1e",
        strip_prefix = "crossbeam-deque-0.8.1",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.crossbeam-deque-0.8.1.bazel"),
    )

    maybe(
        http_archive,
        name = "raze__crossbeam_epoch__0_9_8",
        url = "https://crates.io/api/v1/crates/crossbeam-epoch/0.9.8/download",
        type = "tar.gz",
        sha256 = "1145cf131a2c6ba0615079ab6a638f7e1973ac9c2634fcbeaaad6114246efe8c",
        strip_prefix = "crossbeam-epoch-0.9.8",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.crossbeam-epoch-0.9.8.bazel"),
    )

    maybe(
        http_archive,
        name = "raze__crossbeam_utils__0_8_8",
        url = "https://crates.io/api/v1/crates/crossbeam-utils/0.8.8/download",
        type = "tar.gz",
        sha256 = "0bf124c720b7686e3c2663cf54062ab0f68a88af2fb6a030e87e30bf721fcb38",
        strip_prefix = "crossbeam-utils-0.8.8",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.crossbeam-utils-0.8.8.bazel"),
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
        name = "raze__digest__0_9_0",
        url = "https://crates.io/api/v1/crates/digest/0.9.0/download",
        type = "tar.gz",
        sha256 = "d3dd60d1080a57a05ab032377049e0591415d2b31afd7028356dbf3cc6dcb066",
        strip_prefix = "digest-0.9.0",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.digest-0.9.0.bazel"),
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
        name = "raze__flate2__1_0_23",
        url = "https://crates.io/api/v1/crates/flate2/1.0.23/download",
        type = "tar.gz",
        sha256 = "b39522e96686d38f4bc984b9198e3a0613264abaebaff2c5c918bfa6b6da09af",
        strip_prefix = "flate2-1.0.23",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.flate2-1.0.23.bazel"),
    )

    maybe(
        http_archive,
        name = "raze__fst__0_4_7",
        url = "https://crates.io/api/v1/crates/fst/0.4.7/download",
        type = "tar.gz",
        sha256 = "7ab85b9b05e3978cc9a9cf8fea7f01b494e1a09ed3037e16ba39edc7a29eb61a",
        strip_prefix = "fst-0.4.7",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.fst-0.4.7.bazel"),
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
        name = "raze__generic_array__0_14_5",
        url = "https://crates.io/api/v1/crates/generic-array/0.14.5/download",
        type = "tar.gz",
        sha256 = "fd48d33ec7f05fbfa152300fdad764757cbded343c1aa1cff2fbaf4134851803",
        strip_prefix = "generic-array-0.14.5",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.generic-array-0.14.5.bazel"),
    )

    maybe(
        http_archive,
        name = "raze__hermit_abi__0_1_19",
        url = "https://crates.io/api/v1/crates/hermit-abi/0.1.19/download",
        type = "tar.gz",
        sha256 = "62b467343b94ba476dcb2500d242dadbb39557df889310ac77c5d99100aaac33",
        strip_prefix = "hermit-abi-0.1.19",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.hermit-abi-0.1.19.bazel"),
    )

    maybe(
        http_archive,
        name = "raze__hex__0_4_3",
        url = "https://crates.io/api/v1/crates/hex/0.4.3/download",
        type = "tar.gz",
        sha256 = "7f24254aa9a54b5c858eaee2f5bccdb46aaf0e486a595ed5fd8f86ba55232a70",
        strip_prefix = "hex-0.4.3",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.hex-0.4.3.bazel"),
    )

    maybe(
        http_archive,
        name = "raze__itertools__0_10_3",
        url = "https://crates.io/api/v1/crates/itertools/0.10.3/download",
        type = "tar.gz",
        sha256 = "a9a9d19fa1e79b6215ff29b9d6880b706147f16e9b1dbb1e4e5947b5b02bc5e3",
        strip_prefix = "itertools-0.10.3",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.itertools-0.10.3.bazel"),
    )

    maybe(
        http_archive,
        name = "raze__itoa__1_0_2",
        url = "https://crates.io/api/v1/crates/itoa/1.0.2/download",
        type = "tar.gz",
        sha256 = "112c678d4050afce233f4f2852bb2eb519230b3cf12f33585275537d7e41578d",
        strip_prefix = "itoa-1.0.2",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.itoa-1.0.2.bazel"),
    )

    maybe(
        http_archive,
        name = "raze__json__0_12_4",
        url = "https://crates.io/api/v1/crates/json/0.12.4/download",
        type = "tar.gz",
        sha256 = "078e285eafdfb6c4b434e0d31e8cfcb5115b651496faca5749b88fafd4f23bfd",
        strip_prefix = "json-0.12.4",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.json-0.12.4.bazel"),
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
        name = "raze__libc__0_2_126",
        url = "https://crates.io/api/v1/crates/libc/0.2.126/download",
        type = "tar.gz",
        sha256 = "349d5a591cd28b49e1d1037471617a32ddcda5731b99419008085f72d5a53836",
        strip_prefix = "libc-0.2.126",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.libc-0.2.126.bazel"),
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
        name = "raze__log__0_4_17",
        url = "https://crates.io/api/v1/crates/log/0.4.17/download",
        type = "tar.gz",
        sha256 = "abb12e687cfb44aa40f41fc3978ef76448f9b6038cad6aef4259d3c095a2382e",
        strip_prefix = "log-0.4.17",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.log-0.4.17.bazel"),
    )

    maybe(
        http_archive,
        name = "raze__memchr__2_5_0",
        url = "https://crates.io/api/v1/crates/memchr/2.5.0/download",
        type = "tar.gz",
        sha256 = "2dffe52ecf27772e601905b7522cb4ef790d2cc203488bbd0e2fe85fcb74566d",
        strip_prefix = "memchr-2.5.0",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.memchr-2.5.0.bazel"),
    )

    maybe(
        http_archive,
        name = "raze__memoffset__0_6_5",
        url = "https://crates.io/api/v1/crates/memoffset/0.6.5/download",
        type = "tar.gz",
        sha256 = "5aa361d4faea93603064a027415f07bd8e1d5c88c9fbf68bf56a285428fd79ce",
        strip_prefix = "memoffset-0.6.5",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.memoffset-0.6.5.bazel"),
    )

    maybe(
        http_archive,
        name = "raze__miniz_oxide__0_5_1",
        url = "https://crates.io/api/v1/crates/miniz_oxide/0.5.1/download",
        type = "tar.gz",
        sha256 = "d2b29bd4bc3f33391105ebee3589c19197c4271e3e5a9ec9bfe8127eeff8f082",
        strip_prefix = "miniz_oxide-0.5.1",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.miniz_oxide-0.5.1.bazel"),
    )

    maybe(
        http_archive,
        name = "raze__num_cpus__1_13_1",
        url = "https://crates.io/api/v1/crates/num_cpus/1.13.1/download",
        type = "tar.gz",
        sha256 = "19e64526ebdee182341572e50e9ad03965aa510cd94427a4549448f285e957a1",
        strip_prefix = "num_cpus-1.13.1",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.num_cpus-1.13.1.bazel"),
    )

    maybe(
        http_archive,
        name = "raze__opaque_debug__0_3_0",
        url = "https://crates.io/api/v1/crates/opaque-debug/0.3.0/download",
        type = "tar.gz",
        sha256 = "624a8340c38c1b80fd549087862da4ba43e08858af025b236e509b6649fc13d5",
        strip_prefix = "opaque-debug-0.3.0",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.opaque-debug-0.3.0.bazel"),
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
        name = "raze__path_clean__0_1_0",
        url = "https://crates.io/api/v1/crates/path-clean/0.1.0/download",
        type = "tar.gz",
        sha256 = "ecba01bf2678719532c5e3059e0b5f0811273d94b397088b82e3bd0a78c78fdd",
        strip_prefix = "path-clean-0.1.0",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.path-clean-0.1.0.bazel"),
    )

    maybe(
        http_archive,
        name = "raze__pkg_config__0_3_25",
        url = "https://crates.io/api/v1/crates/pkg-config/0.3.25/download",
        type = "tar.gz",
        sha256 = "1df8c4ec4b0627e53bdf214615ad287367e482558cf84b109250b37464dc03ae",
        strip_prefix = "pkg-config-0.3.25",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.pkg-config-0.3.25.bazel"),
    )

    maybe(
        http_archive,
        name = "raze__proc_macro2__1_0_39",
        url = "https://crates.io/api/v1/crates/proc-macro2/1.0.39/download",
        type = "tar.gz",
        sha256 = "c54b25569025b7fc9651de43004ae593a75ad88543b17178aa5e1b9c4f15f56f",
        strip_prefix = "proc-macro2-1.0.39",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.proc-macro2-1.0.39.bazel"),
    )

    maybe(
        http_archive,
        name = "raze__quick_error__2_0_1",
        url = "https://crates.io/api/v1/crates/quick-error/2.0.1/download",
        type = "tar.gz",
        sha256 = "a993555f31e5a609f617c12db6250dedcac1b0a85076912c436e6fc9b2c8e6a3",
        strip_prefix = "quick-error-2.0.1",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.quick-error-2.0.1.bazel"),
    )

    maybe(
        http_archive,
        name = "raze__quote__1_0_18",
        url = "https://crates.io/api/v1/crates/quote/1.0.18/download",
        type = "tar.gz",
        sha256 = "a1feb54ed693b93a84e14094943b84b7c4eae204c512b7ccb95ab0c66d278ad1",
        strip_prefix = "quote-1.0.18",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.quote-1.0.18.bazel"),
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
        name = "raze__rayon__1_5_3",
        url = "https://crates.io/api/v1/crates/rayon/1.5.3/download",
        type = "tar.gz",
        sha256 = "bd99e5772ead8baa5215278c9b15bf92087709e9c1b2d1f97cdb5a183c933a7d",
        strip_prefix = "rayon-1.5.3",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.rayon-1.5.3.bazel"),
    )

    maybe(
        http_archive,
        name = "raze__rayon_core__1_9_3",
        url = "https://crates.io/api/v1/crates/rayon-core/1.9.3/download",
        type = "tar.gz",
        sha256 = "258bcdb5ac6dad48491bb2992db6b7cf74878b0384908af124823d118c99683f",
        strip_prefix = "rayon-core-1.9.3",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.rayon-core-1.9.3.bazel"),
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
        name = "raze__regex__1_5_5",
        url = "https://crates.io/api/v1/crates/regex/1.5.5/download",
        type = "tar.gz",
        sha256 = "1a11647b6b25ff05a515cb92c365cec08801e83423a235b51e231e1808747286",
        strip_prefix = "regex-1.5.5",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.regex-1.5.5.bazel"),
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
        name = "raze__rls_analysis__0_18_3",
        url = "https://crates.io/api/v1/crates/rls-analysis/0.18.3/download",
        type = "tar.gz",
        sha256 = "85293f293444a5f569dd394024b2af9e91fc6b71d336f35c02bc936a29a5fe7e",
        strip_prefix = "rls-analysis-0.18.3",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.rls-analysis-0.18.3.bazel"),
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
        name = "raze__rls_span__0_5_4",
        url = "https://crates.io/api/v1/crates/rls-span/0.5.4/download",
        type = "tar.gz",
        sha256 = "b6e80f614ad4b37910bfe9b029af19c6f92612bb8e1af66e37d35829bf4ef6d1",
        strip_prefix = "rls-span-0.5.4",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.rls-span-0.5.4.bazel"),
    )

    maybe(
        http_archive,
        name = "raze__ryu__1_0_10",
        url = "https://crates.io/api/v1/crates/ryu/1.0.10/download",
        type = "tar.gz",
        sha256 = "f3f6f92acf49d1b98f7a81226834412ada05458b7364277387724a237f062695",
        strip_prefix = "ryu-1.0.10",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.ryu-1.0.10.bazel"),
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
        name = "raze__serde__1_0_137",
        url = "https://crates.io/api/v1/crates/serde/1.0.137/download",
        type = "tar.gz",
        sha256 = "61ea8d54c77f8315140a05f4c7237403bf38b72704d031543aa1d16abbf517d1",
        strip_prefix = "serde-1.0.137",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.serde-1.0.137.bazel"),
    )

    maybe(
        http_archive,
        name = "raze__serde_derive__1_0_137",
        url = "https://crates.io/api/v1/crates/serde_derive/1.0.137/download",
        type = "tar.gz",
        sha256 = "1f26faba0c3959972377d3b2d306ee9f71faee9714294e41bb777f83f88578be",
        strip_prefix = "serde_derive-1.0.137",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.serde_derive-1.0.137.bazel"),
    )

    maybe(
        http_archive,
        name = "raze__serde_json__1_0_81",
        url = "https://crates.io/api/v1/crates/serde_json/1.0.81/download",
        type = "tar.gz",
        sha256 = "9b7ce2b32a1aed03c558dc61a5cd328f15aff2dbc17daad8fb8af04d2100e15c",
        strip_prefix = "serde_json-1.0.81",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.serde_json-1.0.81.bazel"),
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
        name = "raze__sha2__0_9_9",
        url = "https://crates.io/api/v1/crates/sha2/0.9.9/download",
        type = "tar.gz",
        sha256 = "4d58a1e1bf39749807d89cf2d98ac2dfa0ff1cb3faa38fbb64dd88ac8013d800",
        strip_prefix = "sha2-0.9.9",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.sha2-0.9.9.bazel"),
    )

    maybe(
        http_archive,
        name = "raze__smallvec__1_8_0",
        url = "https://crates.io/api/v1/crates/smallvec/1.8.0/download",
        type = "tar.gz",
        sha256 = "f2dd574626839106c320a323308629dcb1acfc96e32a8cba364ddc61ac23ee83",
        strip_prefix = "smallvec-1.8.0",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.smallvec-1.8.0.bazel"),
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
        name = "raze__syn__1_0_95",
        url = "https://crates.io/api/v1/crates/syn/1.0.95/download",
        type = "tar.gz",
        sha256 = "fbaf6116ab8924f39d52792136fb74fd60a80194cf1b1c6ffa6453eef1c3f942",
        strip_prefix = "syn-1.0.95",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.syn-1.0.95.bazel"),
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
        name = "raze__thiserror__1_0_31",
        url = "https://crates.io/api/v1/crates/thiserror/1.0.31/download",
        type = "tar.gz",
        sha256 = "bd829fe32373d27f76265620b5309d0340cb8550f523c1dda251d6298069069a",
        strip_prefix = "thiserror-1.0.31",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.thiserror-1.0.31.bazel"),
    )

    maybe(
        http_archive,
        name = "raze__thiserror_impl__1_0_31",
        url = "https://crates.io/api/v1/crates/thiserror-impl/1.0.31/download",
        type = "tar.gz",
        sha256 = "0396bc89e626244658bef819e22d0cc459e795a5ebe878e6ec336d1674a8d79a",
        strip_prefix = "thiserror-impl-1.0.31",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.thiserror-impl-1.0.31.bazel"),
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
        name = "raze__typenum__1_15_0",
        url = "https://crates.io/api/v1/crates/typenum/1.15.0/download",
        type = "tar.gz",
        sha256 = "dcf81ac59edc17cc8697ff311e8f5ef2d99fcbd9817b34cec66f90b6c3dfd987",
        strip_prefix = "typenum-1.15.0",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.typenum-1.15.0.bazel"),
    )

    maybe(
        http_archive,
        name = "raze__unicode_ident__1_0_0",
        url = "https://crates.io/api/v1/crates/unicode-ident/1.0.0/download",
        type = "tar.gz",
        sha256 = "d22af068fba1eb5edcb4aea19d382b2a3deb4c8f9d475c589b6ada9e0fd493ee",
        strip_prefix = "unicode-ident-1.0.0",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.unicode-ident-1.0.0.bazel"),
    )

    maybe(
        http_archive,
        name = "raze__unicode_width__0_1_9",
        url = "https://crates.io/api/v1/crates/unicode-width/0.1.9/download",
        type = "tar.gz",
        sha256 = "3ed742d4ea2bd1176e236172c8429aaf54486e7ac098db29ffe6529e0ce50973",
        strip_prefix = "unicode-width-0.1.9",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.unicode-width-0.1.9.bazel"),
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
        name = "raze__version_check__0_9_4",
        url = "https://crates.io/api/v1/crates/version_check/0.9.4/download",
        type = "tar.gz",
        sha256 = "49874b5167b65d7193b8aba1567f5c7d93d001cafc34600cee003eda787e483f",
        strip_prefix = "version_check-0.9.4",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.version_check-0.9.4.bazel"),
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
        name = "raze__zip__0_5_13",
        url = "https://crates.io/api/v1/crates/zip/0.5.13/download",
        type = "tar.gz",
        sha256 = "93ab48844d61251bb3835145c521d88aa4031d7139e8485990f60ca911fa0815",
        strip_prefix = "zip-0.5.13",
        build_file = Label("//kythe/rust/cargo/remote:BUILD.zip-0.5.13.bazel"),
    )
