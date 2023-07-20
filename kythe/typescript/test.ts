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

// This program runs the Kythe verifier on test cases in the testdata/
// directory.  It's written in TypeScript (rather than a plain shell
// script) so it can reuse TypeScript data structures across test cases,
// speeding up the test.
//
// If run with no arguments, runs all tests found in testdata/.
// Otherwise run any files through the verifier by passing them:
//   node test.js path/to/test1.ts testdata/test2.ts

import * as assert from 'assert';
import * as child_process from 'child_process';
import * as fs from 'fs';
import * as path from 'path';
import * as ts from 'typescript';

import * as indexer from './indexer';
import * as kythe from './kythe';
import {CompilationUnit, IndexerHost, Plugin} from './plugin_api';

const KYTHE_PATH = process.env['KYTHE'] || '/opt/kythe';
const RUNFILES = process.env['TEST_SRCDIR'];



const ENTRYSTREAM = RUNFILES ?
    path.resolve('kythe/go/platform/tools/entrystream/entrystream') :
    path.resolve(KYTHE_PATH, 'tools/entrystream');
const VERIFIER = RUNFILES ? path.resolve('kythe/cxx/verifier/verifier') :
                            path.resolve(KYTHE_PATH, 'tools/verifier');

/** Record representing a single test case to run. */
interface TestCase {
  /** Name of the test case. Used for reporting. */
  readonly name: string;
  /**
   * List of files used for indexing. All these files passed together as source
   * files to the indexer.
   */
  readonly files: string[];
}


/**
 * createTestCompilerHost creates a ts.CompilerHost that caches the default
 * libraries.  This prevents re-parsing the (big) TypeScript standard library
 * across each test.
 */
function createTestCompilerHost(options: ts.CompilerOptions): ts.CompilerHost {
  const compilerHost = ts.createCompilerHost(options);

  // Map of path to parsed SourceFile for all TS builtin libraries.
  const libs = new Map<string, ts.SourceFile|undefined>();
  const libDir = compilerHost.getDefaultLibLocation!();

  const hostGetSourceFile = compilerHost.getSourceFile;
  compilerHost.getSourceFile =
      (fileName: string, languageVersion: ts.ScriptTarget,
       onError?: (message: string) => void): ts.SourceFile|undefined => {
        let sourceFile = libs.get(fileName);
        if (!sourceFile) {
          sourceFile = hostGetSourceFile(fileName, languageVersion, onError);
          if (path.dirname(fileName) === libDir) libs.set(fileName, sourceFile);
        }
        return sourceFile;
      };
  return compilerHost;
}

function isTsFile(filename: string): boolean {
  return filename.endsWith('.ts') || filename.endsWith('.tsx');
}

/**
 * verify runs the indexer against a test case and passes it through the
 * Kythe verifier.  It returns a Promise because the node subprocess API must
 * be run async; if there's an error, it will reject the promise.
 */
function verify(
    host: ts.CompilerHost, options: ts.CompilerOptions, testCase: TestCase,
    plugins?: Plugin[], emitRefCallOverIdentifier?: boolean): Promise<void> {
  const rootVName: kythe.VName = {
    corpus: 'testcorpus',
    root: '',
    path: '',
    signature: '',
    language: '',
  };
  const testFiles = testCase.files;

  const verifier = child_process.spawn(
      `${ENTRYSTREAM} --read_format=json | ` +
          `${VERIFIER} --convert_marked_source ${testFiles.join(' ')}`,
      [], {
        stdio: ['pipe', process.stdout, process.stderr],
        shell: true,
      });
  const fileVNames = new Map<string, kythe.VName>();
  for (const file of testFiles) {
    fileVNames.set(file, {...rootVName, path: file});
  }

  try {
    const compilationUnit: CompilationUnit = {
      rootVName,
      fileVNames,
      srcs: testFiles,
      rootFiles: testFiles,
    };
    indexer.index(compilationUnit, {
      compilerOptions: options,
      compilerHost: host,
      emit(obj: {}) {
        verifier.stdin.write(JSON.stringify(obj) + '\n');
      },
      plugins,
      emitRefCallOverIdentifier,
    });
  } finally {
    // Ensure we close stdin on the verifier even on crashes, or otherwise
    // we hang waiting for the verifier to complete.
    verifier.stdin.end();
  }

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

function testLoadTsConfig() {
  const config = indexer.loadTsConfig(
      'testdata/tsconfig-files.for.tests.json', 'testdata');
  // We expect the paths that were loaded to be absolute.
  assert.deepEqual(config.fileNames, [path.resolve('testdata/alt.ts')]);
}

function collectTSFilesInDirectoryRecursively(dir: string, result: string[]) {
  for (const file of fs.readdirSync(dir, {withFileTypes: true})) {
    if (file.isDirectory()) {
      collectTSFilesInDirectoryRecursively(`${dir}/${file.name}`, result);
    } else if (isTsFile(file.name)) {
      result.push(path.resolve(`${dir}/${file.name}`));
    }
  }
}

/**
 * Returns list of files to test. Each group of files will be tested together:
 * all files from a group will be passed to indexer and verifier.
 *
 * The rules of constructing groups are the following:
 * 1. All individual files in testdata/ will be returned as group of one file.
 *    So they are tested separately.
 * 2. All files in subfolders ending with '_group' will be returned as one
 *    group. These are used when cross-file references need to be tested.
 * 3. All individual files in subfolders not ending with '_group' will be
 *    returned as group of one file. So they are tested separately.
 */
function getTestCases(options: ts.CompilerOptions, dir: string): TestCase[] {
  const result = [];
  for (const file of fs.readdirSync(dir, {withFileTypes: true})) {
    const relativeName = `${dir}/${file.name}`;
    if (file.isDirectory()) {
      if (file.name.endsWith('_group')) {
        const files: string[] = [];
        collectTSFilesInDirectoryRecursively(relativeName, files);
        result.push({
          name: relativeName,
          files,
        });
      } else {
        result.push(...getTestCases(options, relativeName));
      }
    } else if (isTsFile(file.name)) {
      result.push({
        name: relativeName,
        files: [path.resolve(relativeName)],
      });
    }
  }
  return result;
}

/**
 * Given filters passed as command line arguments by user - returns only
 * groups that contain at least one file that matches at least one filter.
 * Matching is done by simply checking for substring.
 */
function filterTestCases(testCases: TestCase[], filters: string[]): TestCase[] {
  return testCases.filter(testCase => {
    for (const file of testCase.files) {
      if (filters.some(filter => file.includes(filter))) {
        return true;
      }
    }
    return false;
  });
}

async function testIndexer(filters: string[], plugins?: Plugin[]) {
  const config =
      indexer.loadTsConfig('testdata/tsconfig.for.tests.json', 'testdata');
  let testCases = getTestCases(config.options, 'testdata');
  if (filters.length !== 0) {
    testCases = filterTestCases(testCases, filters);
  }

  const host = createTestCompilerHost(config.options);
  for (const testCase of testCases) {
    if (testCase.name.endsWith('plugin.ts')) {
      // plugin.ts is tested by testPlugin() test.
      continue;
    }
    const emitRefCallOverIdentifier = testCase.name.endsWith('_id.ts');
    const start = new Date().valueOf();
    process.stdout.write(`${testCase.name}: `);
    try {
      await verify(host, config.options, testCase, plugins, emitRefCallOverIdentifier);
    } catch (e) {
      console.log('FAIL');
      throw e;
    }
    const time = new Date().valueOf() - start;
    console.log('PASS', `${time}ms`);
  }
  return 0;
}

async function testPlugin() {
  const plugin: Plugin = {
    name: 'TestPlugin',
    index(context: IndexerHost) {
      for (const testPath of context.compilationUnit.srcs) {
        const pluginMod = {
          ...context.pathToVName(context.moduleName(testPath)),
          signature: 'plugin-module',
          language: 'plugin-language',
        };
        context.options.emit({
          source: pluginMod,
          fact_name: '/kythe/node/pluginKind' as kythe.FactName,
          fact_value: Buffer.from('pluginRecord').toString('base64'),
        });
      }
    },
  };
  return testIndexer(['testdata/plugin.ts'], [plugin]);
}

async function testMain(args: string[]) {
  if (RUNFILES) {
    process.chdir('kythe/typescript');
  }
  testLoadTsConfig();
  await testIndexer(args);
  await testPlugin();
}

testMain(process.argv.slice(2))
    .then(() => {
      process.exitCode = 0;
    })
    .catch((e) => {
      console.error(e);
      process.exitCode = 1;
    });
