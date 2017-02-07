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
