(**
 * Copyright 2015 The Kythe Authors. All rights reserved.
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
 *
 * Derived from flow/src/commands/dumpTypesCommand.ml:
 *
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "flow" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
*)

(***********************************************************************)
(* flow dump-kythe command *)
(***********************************************************************)

module DKS = DumpKytheService_js

open Hh_json
open CommandUtils

let spec = {
  CommandSpec.
  name = "dump-kythe";
  doc = "Dumps Flow data using Kythe format";
  usage = Printf.sprintf
      "Usage: %s dump-kythe [OPTION]... [FILE]\n\n\
       e.g. %s dump-kythe foo.js\n\
       or   %s dump-kythe < foo.js\n"
      CommandUtils.exe_name
      CommandUtils.exe_name
      CommandUtils.exe_name;
  args = CommandSpec.ArgSpec.(
      empty
      |> server_flags
      |> flag "--path" (optional string)
        ~doc:"Specify (fake) path to file when reading data from stdin"
      |> anon "file" (optional string) ~doc:"[FILE]"
    )
}

let get_file path = function
  | Some filename ->
    ServerProt.FileName (expand_path filename)
  | None ->
    let contents = Sys_utils.read_stdin_to_string () in
    let filename = (match path with
        | Some ""
        | None -> None
        | Some str -> Some (get_path_of_file str)
      ) in
    ServerProt.FileContent (filename, contents)

let string_of_pos pos =
  let file = Pos.filename pos in
  if file = Relative_path.default then
    ""
  else
    let line, start, end_ = Pos.info_pos pos in
    if line <= 0 then
      Utils.spf "%s:1:0" (Relative_path.to_absolute file)
    else if Pos.length pos = 1 then
      Utils.spf "%s:%d:%d"
        (Relative_path.to_absolute file) line start
    else
      Utils.spf "%s:%d:%d-%d"
        (Relative_path.to_absolute file) line start end_

let handle_error (pos, err) =
  let pos = Reason_js.string_of_pos pos in
  output_string stderr (Utils.spf "%s:\n%s\n" pos err);
  flush stderr

let main option_values path filename () =
  let file = get_file path filename in
  let root = guess_root (ServerProt.path_of_input file) in
  let ic, oc = connect_with_autostart option_values root in
  ServerProt.cmd_to_channel oc (ServerProt.DUMP_KYTHE file);

  match (Marshal.from_channel ic : DKS.resp_t) with
  | (Some err, None) -> handle_error err
  | (None, Some jsons) -> List.iter
                            (fun j -> print_endline (json_to_string j)) jsons
  | (_, _) -> assert false

let command = CommandSpec.command spec (collect_server_flags main)
