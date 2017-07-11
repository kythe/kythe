/*
 * Copyright 2017 Google Inc. All rights reserved.
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
 */

/**
 * This module is dedicated to mapping local file paths to VNames in Kythe.
 * When a file is opened by the editor, its corresponding file in the Kythe
 * index needs to be located. In Kythe, however, files are facts which are
 * identified by a VName containing a path, a corpus, and an optional root.
 * (The signature field is unused by files). Furthermore, when references are
 * provided by the xref service, these references must be mapped back to local
 * files.
 */

import {join, resolve, relative} from 'path';
import * as p2r from 'path-to-regexp';
import {parse, stringify, unescape} from 'querystring';
import {URL} from 'url';


export interface KytheTicket {
  path?: string;
  root?: string;
  corpus?: string;
  signature?: string;
}

// Convenience type representing a Kythe Ticket encoded as a string
export interface KytheTicketString extends String { _kytheTicketBrand: string; }

// Represents a fully qualified local path on disk
export interface LocalPath extends String { _localPathBrand: string; }

/**
 * PathConfig is a string-based representation of the mapping between
 * local file paths and paths in Kythe. Named parameters are used to
 * to construct one path from the other, meaning all parameters must be
 * present in local and in the vname sections.
 * 
 * Parameters are embedded in paths like so:
 * 
 * 'src/:foo/:bar'
 * 
 * Example:
 * const config = [{
 *    local: 'src/:section/:path*
 *    vname: {
 *        corpus: 'src-:section'
 *        path: ':path*'
 *    }
 * }]
 * 
 * Under this config, we can map like so:
 * Local File: src/dir/path/to/file.js
 * VName: {
 *    corpus: 'src/dir',
 *    path: 'path/to/file.js'
 * }
 * 
 * The first matching config in the array will be used, so order matters.
 */
export type PathConfig = Array<
    {local: string, vname: {corpus?: string, path?: string, root?: string}}>;

type PathMapping = {
  re: p2r.PathRegExp,
  cons: p2r.PathFunction
};

// TODO(djrenren): Remove when config loading is implemented
const DEFAULT_CONFIG: PathConfig =
    [{local: ':file*', vname: {path: 'kythe.io/:file*', corpus: 'kythe.io'}}];

/*
 * Kythe does not require a hard-mapping between local paths and kythe paths.
 * PathContext does this mapping by applying rules defined in a PathConfig.
 */
export class PathContext {

  // An array of mappings to be used when mapping paths to VNames
  private mappings: Array<{
    local: PathMapping,
    vname: {
      corpus: PathMapping,
      path: PathMapping,
      root: PathMapping,
    }
  }>;

  constructor(private root: string, config: PathConfig = DEFAULT_CONFIG) {

    // Normalize our root to have no relative components or trailing slash
    this.root = resolve(root);

    this.mappings = config.map((c) => ({
                                 local: compilePath(c.local),
                                 vname: {
                                   corpus: compilePath(c.vname.corpus || ''),
                                   path: compilePath(c.vname.path || ''),
                                   root: compilePath(c.vname.root || '')
                                 }
                               }));
  }

  // Constructs a KytheTicket by matching the LocalPath against the config
  ticket(localPath: LocalPath): KytheTicket|Error {
    // If the file is not located in our indexed directory it won't be in kythe
    if (!localPath.startsWith(this.root)) {
      return new Error(
          `Path ${localPath} does not start with root ${this.root}`);
    }

    // All config filepaths are relative to root so we need to truncate
    const truncPath = relative(this.root, localPath as String as string);

    for (const {local, vname: {path, corpus, root}} of this.mappings) {
      const params = extractParams(local.re, truncPath);
      if (params === null) continue;

      return {
        // path-to-regexp escapes slashes for us but we don't want that.
        path: unescape(path.cons(params)),
        corpus: corpus.cons(params),
        root: root.cons(params)
      };
    }
    return new Error(`No matching config found for ${localPath}`);
  }

  // Converts a KytheTicket into a LocalPath
  local(ticket: KytheTicket): LocalPath|Error {
    for (const {local, vname: {path, corpus, root}} of this.mappings) {

      const pathParams = extractParams(path.re, ticket.path || '');
      const corpusParams =
          extractParams(corpus.re, ticket.corpus || '');
      const rootParams =
          extractParams(root.re, ticket.root || '');
      
      // If any of the params are null, the match failed
      if (pathParams === null || corpusParams === null || rootParams === null) {
        continue;
      }

      // Combine params from all elements of the ticket
      const params = {...pathParams, ...corpusParams, ...rootParams};

      // If the object is still empty, we failed to match
      if (Object.keys(params).length === 0) continue;

      // Add the root directory as a prefix to become fully qualified
      const localPath = join(this.root, unescape(local.cons(params)));

      return localPath as String as LocalPath;
    }

    return new Error(
        `No matching config found for ticket: ${JSON.stringify(ticket)}`);
  }
}

// Converts a KytheTicketString into a KytheTicket
export function parseTicket(ticket: KytheTicketString): KytheTicket {
  const url = new URL(ticket as String as string);
  return {corpus: url.host, ...parse(url.search, '?')};
}

export function tryParseTicket(s: string): KytheTicket|Error {
  try {
    const ticket = parseTicket(s as String as KytheTicketString);
    if (ticket.path === undefined)
      return new Error('Ticket String was well-shaped but underspecified');
    return ticket;
  } catch (e) {
    return e;
  }
}

// Normalizes language server provided paths
// TODO(djrenren): Define the supported types of URLs. Currently git and file
export function normalizeLSPath(path: string): LocalPath {
  const uri = new URL(path);

  if (uri.protocol === 'git:' && uri.protocol.endsWith('.git')) {
    // Remove the .git extension
    return uri.pathname.substring(0, uri.pathname.length - 4) as String as
        LocalPath;
  }

  return uri.pathname as String as LocalPath;
}

// Produces a KytheTicketString from a given KytheTicket
export function ticketString({corpus, ...params}: KytheTicket) {
  if (params.root === '') delete params.root;
  return `kythe://${corpus}?` + stringify(params, '?') as String as
      KytheTicketString;
}



type Params = {
  [id: string]: string
};

// Take a parameterized route regexp and match against a path producing an
// object keyed by the params
function extractParams(re: p2r.PathRegExp, path: String): Params|null {
  // Prefix the strings we're matching against with a slash so they adhere to
  // what path-to-regexp expects
  path = '/' + path;

  const match = re.exec(path as string);
  if (!match) return null;

  const params = {} as Params;

  match.shift();  // match[0] is the full string which we don't want
  match.forEach((s, i) => {
    params[re.keys[i].name] = s;
  });

  return params;
}

function compilePath(path: string): PathMapping {
  // Prefix all the path patterns with / because path-to-regexp requires one
  path = '/' + path;

  const cons = p2r.compile(path);

  // Because the patterns were prefixed with slashes, we remove them on construction
  return {re: p2r(path), cons: (params: Params) => cons(params).substring(1)};
}
