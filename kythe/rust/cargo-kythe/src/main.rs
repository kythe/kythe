// Copyright 2016 The Kythe Authors. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

use std::path::Path;
use std::process::{Command, Stdio, Child};
use std::{fs, env};

static GRAPH_DIR: &'static str = "target/kythe/gs";
static TABLE_DIR: &'static str = "target/kythe/tables";

fn main() {
    let kythe_dir = env::var("KYTHE_DIR").expect("KYTHE_DIR environment variable must be set");

    // Skip the executable name and cargo command "kythe"
    let mut args = env::args().skip(2);

    if let Some(cmd) = args.next() {
        match cmd.as_ref() {
            "full-index" => full_index(kythe_dir.as_ref()),
            "index" => index(kythe_dir.as_ref()),
            "web" => web(kythe_dir.as_ref()),
            _ => panic!("Unknown command"),
        }
    } else {
        index(kythe_dir.as_ref());
    }
}

fn full_index(kythe_dir: &Path) {
    Command::new("cargo").arg("clean").spawn().unwrap().wait().unwrap();
    index(kythe_dir);
}

fn index(kythe_dir: &Path) {
    let so_folder = Path::new(env!("CARGO_HOME")).join("kythe");

    let rustc_flags = env::var("RUSTFLAGS").unwrap_or(String::new());

    let rustc_flags = format!("{} -L {} -Zextra-plugins=kythe_indexer ",
                              rustc_flags,
                              so_folder.to_string_lossy());

    env::set_var("RUSTFLAGS", rustc_flags);
    let indexer = Command::new("cargo")
        .arg("build")
        .args(&["-j", "1"])
        .stdout(Stdio::piped())
        .spawn()
        .expect("failed to execute rustc");

    let stdout = stdio_from_child(&indexer);

    let entry_stream = Command::new(kythe_dir.join("tools/entrystream"))
        .arg("--read_format=json")
        .stdin(stdout)
        .stdout(Stdio::piped())
        .spawn()
        .expect("could not start entrystream");
    let stdout = stdio_from_child(&entry_stream);

    let _ = fs::remove_dir_all(GRAPH_DIR);
    let _ = fs::remove_dir_all(TABLE_DIR);
    fs::create_dir_all(GRAPH_DIR).unwrap();
    fs::create_dir_all(TABLE_DIR).unwrap();

    let write_entries = Command::new(kythe_dir.join("tools/write_entries"))
        .args(&["--workers", "12", "--graphstore", GRAPH_DIR])
        .stdin(stdout)
        .spawn()
        .expect("could not start write_entries");

    [indexer, entry_stream, write_entries].iter_mut().all(|child| child.wait().unwrap().success());

    let mut write_tables = Command::new(kythe_dir.join("tools/write_tables"))
        .args(&["-graphstore", GRAPH_DIR, "-out", TABLE_DIR])
        .spawn()
        .expect("could not start write_tables");

    write_tables.wait().unwrap();
}


fn stdio_from_child(child: &Child) -> Stdio {
    unsafe {
        use std::os::unix::io::FromRawFd;
        use std::os::unix::io::AsRawFd;
        Stdio::from_raw_fd(child.stdout.as_ref().unwrap().as_raw_fd())
    }
}

fn web(kythe_dir: &Path) {

    let mut http_server = Command::new(kythe_dir.join("tools/http_server"))
        .args(&["-serving_table",
                TABLE_DIR,
                "-public_resources",
                &kythe_dir.join("web/ui").to_string_lossy(),
                "-grpc_listen",
                ":8081",  // "localhost:8081" allows access from only this machine
                "-listen",
                ":8080"  // "localhost:8080" allows access from only this machine
                ])
        .spawn()
        .expect("could not start http_server");

    http_server.wait().unwrap();
}
