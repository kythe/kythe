/*
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
 */

var fs = require('fs');
var gulp = require('gulp');
var gutil = require('gulp-util');
var merge = require('merge2');
var ts = require('gulp-typescript');
var typescript = require('typescript');

var tsProject = ts.createProject({
  module: "commonjs",
  // We're using a branch of typescript in third_party.
  noExternalResolve: false,
  noImplicitAny: true,
  declarationFiles: true,
  noEmitOnError: true,
  typescript: typescript
});

var hasError;
var failOnError = true;

var onError = function(err) {
  hasError = true;
  gutil.log(err.message);
  if (failOnError) {
    process.exit(1);
  }
};

gulp.task('compile', function() {
  hasError = false;
  var tsResult =
      gulp.src(['node_modules/typescript/lib/typescript.d.ts', 'lib/*.ts',
                'typings/**/*.d.ts'])
          .pipe(ts(tsProject))
          .on('error', onError);
  return merge([
    tsResult.dts.pipe(gulp.dest('build/definitions')),
    tsResult.js.pipe(gulp.dest('build/lib')),
  ]);
});

gulp.task('default', ['compile']);
