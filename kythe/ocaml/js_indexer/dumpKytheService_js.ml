(**
 * Copyright 2015 Google Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *)

(***********************************************************************)
(* flow server-side implementation for the dump-kythe command *)
(***********************************************************************)

type resp_t = (Pos.t * string) option * Hh_json.json list option

let query file_input : resp_t =
  (* Make sure we have the B64 module available. *)
  let _ = B64.encode "..." in
  (None, Some [])
