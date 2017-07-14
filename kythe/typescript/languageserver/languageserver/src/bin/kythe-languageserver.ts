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

import * as fs from 'fs';
import {join} from 'path';
import {URL} from 'url';
import {createConnection, IConnection, InitializeResult} from 'vscode-languageserver';

import {PathContext} from '../pathContext';
import {Server} from '../server';
import {parseSettings} from '../settings';
import {XRefHTTPClient} from '../xrefClient';

const SETTINGS_FILE = '.kythe-settings.json';

const conn: IConnection = createConnection();



conn.onInitialize((params): InitializeResult => {
  const root = params.rootUri ? new URL(params.rootUri).pathname : '';
  const settingsPath = join(root, SETTINGS_FILE);

  try {
    fs.accessSync(settingsPath, fs.constants.R_OK);
  } catch (_) {
    conn.window.showErrorMessage(
        `${SETTINGS_FILE} not found in project root '${root}'`);
    // If we cannot find the settings file, we have no capabilities
    return {capabilities: {}};
  }

  const settingsObject = JSON.parse(fs.readFileSync(settingsPath, 'UTF8'));

  const settings = parseSettings(settingsObject);
  if (settings instanceof Error) {
    conn.window.showErrorMessage(settings.message);
    return {capabilities: {}};
  }

  const server = new Server(
      new PathContext(root, settings.mappings),
      new XRefHTTPClient(settings.xrefs.host, settings.xrefs.port));

  const ret = server.onInitialize(params);

  /* All implemented behaviors go here */
  conn.onDidOpenTextDocument(server.onDidOpenTextDocument.bind(server));
  conn.onReferences(server.onReferences.bind(server));
  conn.onDefinition(server.onDefinition.bind(server));

  return ret;
});

console.error('LISTENING!');
conn.listen();
