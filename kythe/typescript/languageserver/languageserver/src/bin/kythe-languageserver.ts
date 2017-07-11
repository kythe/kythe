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

import {URL} from 'url';
import {createConnection, IConnection, InitializeResult} from 'vscode-languageserver';

import {PathContext} from '../pathContext';
import {Server} from '../server';
import {XRefHTTPClient} from '../xrefClient';

const conn: IConnection = createConnection();

conn.onInitialize((params): InitializeResult => {
  const root = params.rootUri ? new URL(params.rootUri).pathname : '';

  const server =
      new Server(new PathContext(root), new XRefHTTPClient('localhost', 8080));

  const ret = server.onInitialize(params);

  /* All implemented behaviors go here */
  conn.onDidOpenTextDocument(server.onDidOpenTextDocument.bind(server));
  conn.onReferences(server.onReferences.bind(server));

  return ret;
});

conn.listen();
