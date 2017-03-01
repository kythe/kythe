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

// This program runs the Kythe verifier on test cases in the testdata/
// directory.  It's written in TypeScript (rather than a plain shell)
// script) so it can reuse TypeScript data structures across test cases,
// speeding up the test.

import * as child_process from 'child_process';
import * as fs from 'fs';
import * as ts from 'typescript';

import * as indexer from './indexer';

const KYTHE_PATH = '/opt/kythe';

let tsOptions: ts.CompilerOptions = {
  // Disable searching for @types typings.  This prevents TS from looking around
  // for a node_modules directory.
  types: [],
  lib: ['lib.es6.d.ts'],
};

/**
 * createTestCompilerHost creates a ts.CompilerHost that caches its default
 * library.  This prevents re-parsing the (big) TypeScript standard library
 * across each test.
 */
function createTestCompilerHost(): ts.CompilerHost {
  let compilerHost = ts.createCompilerHost(tsOptions);

  let libPath = compilerHost.getDefaultLibFileName(tsOptions);
  let libSource = compilerHost.getSourceFile(libPath, ts.ScriptTarget.ES2015);

  let hostGetSourceFile = compilerHost.getSourceFile;
  compilerHost.getSourceFile =
      (fileName: string, languageVersion: ts.ScriptTarget,
       onError?: (message: string) => void): ts.SourceFile => {
        if (fileName === libPath) return libSource;
        return hostGetSourceFile(fileName, languageVersion, onError);
      };
  return compilerHost;
}

/**
 * verify runs the indexer against a test case and passes it through the
 * Kythe verifier.  It returns a Promise because the node subprocess API must
 * be run async; if there's an error, it will reject the promise.
 */
function verify(host: ts.CompilerHost, test: string): Promise<void> {
  let program = ts.createProgram([test], tsOptions, host);

  let verifier = child_process.spawn(
      `${KYTHE_PATH}/tools/entrystream --read_json |` +
          `${KYTHE_PATH}/tools/verifier ${test}`,
      [], {
        stdio: ['pipe', process.stdout, process.stderr],
        shell: true,
      });

  indexer.index([test], program, (obj: any) => {
    verifier.stdin.write(JSON.stringify(obj) + '\n');
  });
  verifier.stdin.end();

  return new Promise<void>((resolve, reject) => {
    verifier.on('close', (exitCode) => {
      if (exitCode === 0) {
        resolve();
      } else {
        reject(`process exited with code ${exitCode}`);
      }
    });
  });
}

async function testMain() {
  // chdir into the testdata directory so that the compiler doesn't see the
  // node_modules contained in this project.
  process.chdir('testdata');

  let host = createTestCompilerHost();
  for (const test of fs.readdirSync('.')) {
    if (!test.match(/\.ts$/)) continue;

    let start = new Date().valueOf();
    process.stdout.write(`${test}: `);
    try {
      await verify(host, test);
    } catch (e) {
      console.log('FAIL');
      throw e;
    }
    let time = new Date().valueOf() - start;
    console.log('PASS', time + 'ms');
  }
  return 0;
}

testMain()
    .then(() => process.exit(0))
    .catch((e) => {
      console.error(e);
      process.exit(1);
    });
