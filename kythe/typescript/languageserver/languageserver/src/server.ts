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

import {StringDecoder} from 'string_decoder';
import * as LS from 'vscode-languageserver';

import {Document, RefResolution} from './document';
import {KytheTicketString, LocalPath, normalizeLSPath, PathContext, ticketString, tryParseTicket} from './pathContext';
import {kythe} from './proto/xref';
import {XRefClient} from './xrefClient';


export class Server {
  // Used to find the Document containing file decorations for a given LocalPath
  private kytheDocuments: Map<LocalPath, Document> =
      new Map<LocalPath, Document>();

  constructor(private paths: PathContext, private client: XRefClient) {}

  onInitialize(_params: LS.InitializeParams): LS.InitializeResult {
    return {
      capabilities: {
        referencesProvider: true,
        textDocumentSync: LS.TextDocumentSyncKind.Full,
        definitionProvider: true,
      }
    };
  }

  async onReferences({textDocument: {uri}, position}: LS.ReferenceParams):
      Promise<LS.Location[]> {
    const localPath = normalizeLSPath(uri);
    const doc = this.kytheDocuments.get(localPath);

    // If we don't have decorations for the file, we can't find references
    if (!doc) return [];

    const ticket = doc.xrefs(position) as {} as string;
    const xrefs = await this.client.xrefs({
      ticket: [ticket],
      reference_kind:
          kythe.proto.CrossReferencesRequest.ReferenceKind.NON_CALL_REFERENCES,
      definition_kind:
          kythe.proto.CrossReferencesRequest.DefinitionKind.BINDING_DEFINITIONS,
      declaration_kind:
          kythe.proto.CrossReferencesRequest.DeclarationKind.ALL_DECLARATIONS,
    });

    if (xrefs.cross_references == null) return [];
    const refs = xrefs.cross_references[ticket].reference || [];

    const locs =
        refs.map(r => this.anchortoLoc(r)).filter(r => !(r instanceof Error)) as
        LS.Location[];

    this.adjustDirtyLocations(locs);
    return locs;
  }

  async onDidChangeTextDocument({
    textDocument: {uri: path},
    contentChanges: [{text}]
  }: LS.DidChangeTextDocumentParams): Promise<void> {
    const localPath = normalizeLSPath(path);
    const doc = this.kytheDocuments.get(localPath);
    if (doc === undefined) {
      return;
    }

    doc.updateDirtyState(text);
  }

  async onDidOpenTextDocument({textDocument: {uri: path, text}}:
                                  LS.DidOpenTextDocumentParams): Promise<void> {
    const localPath = normalizeLSPath(path);
    const kytheTicket = this.paths.ticket(localPath);

    if (kytheTicket instanceof Error) {
      console.error(kytheTicket.message);
      return;
    }

    if (this.kytheDocuments.has(localPath)) {
      return;
    }

    const qualifiedXRefs: RefResolution[] = [];
    const ticketStr = ticketString(kytheTicket);
    const dec = await this.client.decorations({
      location: {ticket: ticketStr as String as string},
      references: true,
      target_definitions: true,
      source_text: true
    });

    for (const r of dec.reference || []) {
      if (r.span === undefined) continue;

      const range = spanToRange(r.span);
      if (range instanceof Error) {
        console.error(range.message);
        continue;
      }

      qualifiedXRefs.push(
          {target: r.target_ticket as String as KytheTicketString, range});
    }

    if (dec.source_text === undefined) {
      console.error(`Server failed to provide source_text for ${ticketStr}`);
      return;
    }
    const decoder = new StringDecoder('utf8');
    const textBuffer = Buffer.from(dec.source_text as {} as string, 'base64');
    const kytheText = decoder.write(textBuffer);
    const doc = new Document(qualifiedXRefs, kytheText, text);
    this.kytheDocuments.set(localPath, doc);
  }

  async onDefinition({textDocument: {uri},
                      position}: LS.TextDocumentPositionParams):
      Promise<LS.Location[]> {
    const localPath = normalizeLSPath(uri);
    const doc = this.kytheDocuments.get(localPath);

    // If we don't have decorations for the file, we can't find references
    if (!doc) return [];

    const ticket = doc.xrefs(position) as {} as string;

    // LSP does not distinguish definitions and declarations so just return both
    const xrefs = await this.client.xrefs({
      ticket: [ticket],
      definition_kind:
          kythe.proto.CrossReferencesRequest.DefinitionKind.BINDING_DEFINITIONS,
      declaration_kind:
          kythe.proto.CrossReferencesRequest.DeclarationKind.ALL_DECLARATIONS,
    });

    if (xrefs.cross_references == null) return [];
    const refs = [
      ...xrefs.cross_references[ticket].declaration || [],
      ...xrefs.cross_references[ticket].definition || []
    ];

    const locs =
        refs.map(r => this.anchortoLoc(r)).filter(r => !(r instanceof Error)) as
        LS.Location[];

    this.adjustDirtyLocations(locs);

    return locs;
  }


  private anchortoLoc(r: kythe.proto.CrossReferencesReply.IRelatedAnchor):
      LS.Location|Error {
    if (r.anchor === undefined || r.anchor.parent === undefined ||
        r.anchor.span === undefined || r.anchor.parent === undefined) {
      return new Error('Anchor underspecified: ' + JSON.stringify(r));
    }

    const range = spanToRange(r.anchor.span);
    if (range instanceof Error) {
      return range;
    }

    const ticket = tryParseTicket(r.anchor.parent);
    if (ticket instanceof Error) {
      return ticket;
    }

    const local = this.paths.local(ticket);
    if (local instanceof Error) {
      return local;
    }

    return {
      uri: 'file://' + local,
      range,
    };
  }

  // Takes a list of locations in original source files and changes them to the
  // corresponding location in the dirty state
  private adjustDirtyLocations(locs: LS.Location[]) {
    for (const l of locs) {
      const localPath = normalizeLSPath(l.uri);
      const doc = this.kytheDocuments.get(localPath);
      // If the document isn't open, it doesn't have a dirty state we can work
      // with Note: The document may still be dirty and not open. Checking this
      // is too expensive.
      if (doc === undefined) {
        continue;
      }

      const dirtyRange = doc.dirtyRange(l.range);
      if (dirtyRange instanceof Error) {
        console.error(dirtyRange);
        continue;
      }

      l.range = dirtyRange;
    }
  }
}



function spanToRange(s: kythe.proto.common.ISpan): LS.Range|Error {
  if (s.start === undefined || s.end === undefined) {
    return new Error('Span underspecified: ' + JSON.stringify(s));
  }

  return {
    start: {
      line: (s.start.line_number || 1) - 1,
      character: s.start.column_offset || 0
    },
    end: {
      line: (s.end.line_number || 1) - 1,
      character: s.end.column_offset || 0
    }
  };
}
