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
*)

(***********************************************************************)
(* flow server-side implementation for the dump-kythe command *)
(***********************************************************************)

open Constraint_js
open Hh_json
open Spider_monkey_ast
open Utils

module TI = Type_inference_js

type vname = {
  signature: string;
  path: string;
  language : string;
  root: string;
  corpus: string;
}
let null_vname =
  { signature = ""; path = ""; language = ""; root = ""; corpus = "" }
let js_vname =
  { signature = ""; path = ""; language = "js"; root = ""; corpus = "" }

(* A Kythe entry: source, edge, destination, fact name, fact value. *)
type entry = (vname * string * vname option * string * string)

let json_of_vname vname =
  JAssoc [
    ("signature", JString vname.signature);
    ("path", JString vname.path);
    ("corpus", JString vname.corpus);
    ("root", JString vname.root);
    ("language", JString vname.language)
  ]

let json_of_entry (src, edge, tgt, fname, fval) =
  let assocs = [
    ("source", json_of_vname src);
    ("edge_kind", JString edge);
    ("fact_name", JString fname);
    ("fact_value", JString (B64.encode fval))
  ] in
  match tgt with
  | None -> JAssoc assocs
  | Some v -> JAssoc (("target", json_of_vname v) :: assocs)

let fact node fact_name fact_val =
  (node, "", None, "/kythe/" ^ fact_name, fact_val)
let edge src edge_name tgt =
  (src, "/kythe/edge/" ^ edge_name, Some tgt, "/", "")
let edge_p src edge_name tgt p =
  (src, "/kythe/edge/" ^ edge_name, Some tgt, "/", p)

(* We assume for now that every file is a member of some npm module.
 * We'll use that module's name as the corpus. (Note that the current module
 * is available from the context using Module_js.info_of) *)
let vname_of_path path =
  let info = Module_js.get_module_info path in
  { signature = ""; path = path; language = "js"; root = "";
    corpus = info.Module_js._module }

(* If p is valid, return a tuple of p's file's VName and the VName of the
 * anchor covering p. *)
let path_anchor_vname (p:Loc.t) =
  match p.Loc.source with
  | None -> None
  | Some path ->
    let path_vname = vname_of_path path in
    (* We use line and col here because there seems to be a bug inside Flow that
     * causes it to drop pos_bol (and internally, offsets in Loc are calculated
     * starting from line/col/bol in Pos anyhow). *)
    let start_l = p.Loc.start.Loc.line in
    let _end_l = p.Loc._end.Loc.line in
    let start_c = p.Loc.start.Loc.column in
    let _end_c = p.Loc._end.Loc.column in
    let new_sig = (Printf.sprintf "%d-%d-%d-%d%s"
                     start_l start_c _end_l _end_c path_vname.signature) in
    let vname = {path_vname with signature = new_sig} in
    Some (path_vname, vname)

(* If p is invalid, return (es, None); otherwise, return the entries
 * establishing an anchor node at p prepended to es and that anchor node's
 * VName. *)
let anchor (p:Loc.t) es =
  match path_anchor_vname p with
  | None -> (es, None)
  | Some (path_vname, vname) ->
    let start = p.Loc.start.Loc.offset in
    let _end = p.Loc._end.Loc.offset in
    (json_of_entry (fact vname "node/kind" "anchor") ::
     json_of_entry (fact vname "loc/start" (string_of_int start)) ::
     json_of_entry (fact vname "loc/end" (string_of_int _end)) ::
     json_of_entry (edge vname "childof" path_vname) :: es, Some vname)

(* A use site that we were notified about by Flow. Since we don't have access
 * to scope information after typechecking is complete, we rely on callbacks
 * to fill out a table of mappings from reference locations to referents.
 * A discovered_ref is a referent that we've been told about, but that may
 * not yet at that time be fully constrained. *)
type discovered_ref =
  (* The referent was defined at a simple source location (e.g., a variable
   * definition). *)
  | DrLoc of string * Spider_monkey_ast.Loc.t
  (* The referent was a string identifier key in some given `this` type;
   * it was discovered as part of a function call. *)
  | DrCall of string * Constraint_js.Type.t
  (* The referent was a string identifier key in some given `this` type;
   * it was discovered as part of member access. *)
  | DrMember of string * Constraint_js.Type.t

(* Called by Flow when the typechecker discovers a call at loc to a key
 * called name in type `this`. *)
let call_hook ref_table cxt name loc this =
  Hashtbl.add ref_table loc (DrCall (name, this))

(* Called by Flow when the typechecker discovers a reference at loc to a
 * key called name in type `this`. *)
let member_hook ref_table cxt name loc this =
  Hashtbl.add ref_table loc (DrMember (name, this)); false

(* Called by Flow when the typechecker discovers a reference at loc to an
 * identifier called name. This hook looks in the current scope to get the
 * definition location of loc and insert it into ref_table, where this is
 * possible. *)
let id_hook ref_table cxt name loc =
  let env = Env_js.all_entries () in
  match Utils.SMap.get name env with
  | Some {Scope.def_loc;_} -> (
      match def_loc with
      | Some loc' -> (
          Hashtbl.add ref_table loc (DrLoc (name, loc'));
          false
        )
      | None -> false
    )
  | None -> false

(* A resolved reference: its identifier, use location, and def location. *)
type finalized_ref = (string * Pos.t * Pos.t)

(* Builds a finalized_ref from a context, use location, and discovered_ref. *)
let finalize_ref cx loc kind =
  match kind with
  | DrLoc (v, loc') -> Some (v, loc, loc')
  | DrCall (name, this)
  | DrMember (name, this) ->
    let this_t = Flow_js.resolve_type cx this in
    try let result_map = Flow_js.extract_members cx this_t
      in match Utils.SMap.get name result_map with
      | Some t
        (* loc_of_t t points to the initializing expression *)
        -> Some (name, loc, Constraint_js.loc_of_t t)
      | None -> None
    with Not_found -> None

(* Prepends all the entries belonging to the file node for path to es. *)
let file_entries_of_path path es =
  let file_in = open_in path in
  let file_len = in_channel_length file_in in
  let file_content = String.create file_len in
  really_input file_in file_content 0 file_len;
  close_in file_in;
  json_of_entry (fact (vname_of_path path) "text" file_content) ::
  json_of_entry (fact (vname_of_path path) "node/kind" "file") ::
  es

(* Dumps Flow's information for path using the context cx and the provided
 * reference table. *)
let dump_xrefs path cx ref_table =
  let es = file_entries_of_path path [] in
  Hashtbl.fold (fun k v es ->
      match finalize_ref cx k v with
      | None -> es
      | Some (name, use_loc, def_loc) ->
        let (es, use_vname') = anchor use_loc es in
        let (es, def_vname') = anchor def_loc es in
        match (use_vname', def_vname') with
        | (Some use_vname, Some def_vname) ->
          let def_tgt =
            { def_vname with signature = name ^ "#" ^ def_vname.signature } in
          json_of_entry (edge use_vname "ref" def_tgt) ::
          json_of_entry (edge def_vname "defines" def_tgt) ::
          json_of_entry (fact def_tgt "node/kind" "js/todo") :: es
        (* Ignore those cases where the locations are invalid. *)
        | _ -> es
    ) ref_table es

(* RPC response to the query: a list of errors and a list of JSON entries. *)
type resp_t = (Pos.t * string) option * Hh_json.json list option

let mk_pos file line col =
  {
    Pos.
    pos_file = Relative_path.create Relative_path.Dummy file;
    pos_start = Reason_js.lexpos file line col;
    pos_end = Reason_js.lexpos file line (col+1);
  }

(* Emit Flow's inferred data about file_input as JSON-encoded Kythe entries. *)
let query file_input : resp_t =
  let table : (Spider_monkey_ast.Loc.t, discovered_ref) Hashtbl.t =
    Hashtbl.create 0 in
  Type_inference_hooks_js.set_id_hook (id_hook table);
  Type_inference_hooks_js.set_member_hook (member_hook table);
  Type_inference_hooks_js.set_call_hook (call_hook table);
  let file = ServerProt.file_input_get_filename file_input in
  try
    let cx = match file_input with
      | ServerProt.FileName file ->
        let content = ServerProt.file_input_get_content file_input in
        (match Types_js.typecheck_contents content file with
         | Some cx, _ -> cx
         | _, errors -> failwith "Couldn't parse file")
      | ServerProt.FileContent (_, content) ->
        (match Types_js.typecheck_contents content file with
         | Some cx, _ -> cx
         | _, errors  -> failwith "Couldn't parse file") in
    Type_inference_hooks_js.reset_hooks();
    (None, Some (dump_xrefs file cx table))
  with exn ->
    let pos = mk_pos file 0 0 in
    let err = (pos, Printexc.to_string exn) in
    (Some err, None)
