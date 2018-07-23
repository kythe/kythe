/*
 * Copyright 2017 The Kythe Authors. All rights reserved.
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

import {join} from 'path';
import {Disposable, ExtensionContext, window, workspace} from 'vscode';
import {LanguageClient, LanguageClientOptions, ServerOptions, TransportKind} from 'vscode-languageclient';


// This method is called when the extension is activated
export function activate(context: ExtensionContext) {
  const debugOptions = {execArgv: ['--nolazy', '--debug=6004']};
  const settings = workspace.getConfiguration('kytheLanguageServer');

  const serverOptions: ServerOptions = {
    command: settings.get('bin') || 'kythe_languageserver',
    args: settings.get('args'),
  };

  const clientOptions: LanguageClientOptions = {
    // Activate on all files and defer to the server to ignore stuff
    documentSelector: [{pattern: '**/*'}]
  }

  const disposable = new LanguageClient(
                         'kytheLanguageServer', 'Kythe Language Server',
                         serverOptions, clientOptions)
                         .start();

  context.subscriptions.push(disposable);
}
