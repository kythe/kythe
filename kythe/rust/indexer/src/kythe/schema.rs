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
use std::fmt;
use std::ops::Deref;

#[derive(Default, Clone, Debug, RustcEncodable)]
pub struct VName {
    pub signature: Option<String>,
    pub corpus: Option<String>,
    pub root: Option<String>,
    pub path: Option<String>,
    pub language: Option<String>,
}

// A macro for string-based enums that derives Deref<Target=str> and Display
macro_rules! str_enum {
    ($enum_id:ident { $($val_id:ident => $val:expr,)+ }) => {
        pub enum $enum_id {
            $(
                $val_id,
            )*
        }
        impl fmt::Display for $enum_id {
            fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
                let out = match self {
                    $(&$enum_id::$val_id => $val,)*
                };
                write!(f, "{}", out)
            }
        }
        impl Deref for $enum_id {
            type Target = str;
            fn deref(&self) -> &str {
                match self {
                    $(&$enum_id::$val_id => $val,)*
                }
            }
        }
    }
}

str_enum! {
    NodeKind {
       File => "file",
       Anchor => "anchor",
       Function => "function",
       Variable => "variable",
       Constant => "constant",
    }
}

str_enum! {
    EdgeKind {
        ChildOf => "/kythe/edge/childof",
        Defines => "/kythe/edge/defines",
        DefinesBinding => "/kythe/edge/defines/binding",
        Ref => "/kythe/edge/ref",
        RefCall => "/kythe/edge/ref/call",
    }
}

str_enum! {
    Complete {
        Definition => "definition",
    }
}

str_enum! {
    Fact {
        NodeKind => "/kythe/node/kind",
        LocStart => "/kythe/loc/start",
        LocEnd => "/kythe/loc/end",
        Text => "/kythe/text",
        Complete => "/kythe/complete",
    }
}
