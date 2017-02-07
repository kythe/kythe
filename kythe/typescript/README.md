# Kythe indexer for TypeScript

## Development

Install [yarn](https://yarnpkg.com/), then run it with no arguments to download
dependencies.

Run `yarn run build` to compile the TypeScript once.

Run `yarn run watch` to start the TypeScript compiler in watch mode, which
keeps the built program up to date.  Use this while developing.

Run `yarn run browse` to run the main binary, which opens the Kythe browser
against a sample file.  (You might need to set `$KYTHE` to your Kythe path
first.)

Run `yarn test` to run the test suite.  (You'll need to have built first.)

### Writing tests

By default in TypeScript, files are "scripts", where every declaration is in
the global scope.  If the file has any `import` or `export` declaration, they
become a "module", where declarations are local.  To make tests isolated from
one another, prefix each test with an `export {}` to make them modules.  In
larger TypeScript projects this doesn't come up because all files are modules.