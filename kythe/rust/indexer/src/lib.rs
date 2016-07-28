// Copyright 2016 Google Inc. All rights reserved.
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

#![feature(plugin_registrar)]
#![feature(slice_patterns, box_syntax, rustc_private)]

extern crate syntax;
extern crate rustc_serialize;
// Load rustc as a plugin to get macros.
#[macro_use]
extern crate rustc;
extern crate rustc_plugin;

mod kythe;
mod pass;
mod visitor;

use kythe::corpus::Corpus;
use kythe::writer::JsonEntryWriter;
use rustc_plugin::Registry;
use rustc::lint::LateLintPassObject;
use std::env;

// Informs the compiler of the existence and implementation of our plugin.
#[plugin_registrar]
pub fn plugin_registrar(reg: &mut Registry) {
    let pass = box pass::KytheLintPass::new(get_corpus(), box JsonEntryWriter);
    reg.register_late_lint_pass(pass as LateLintPassObject);
}

// Reads the corpus name from the environment variable.
// Corpus name will default to the empty string is the variable is not present.
fn get_corpus() -> Corpus {
    let corpus_name = env::var("KYTHE_CORPUS").unwrap_or(String::new());
    Corpus { name: corpus_name }
}
