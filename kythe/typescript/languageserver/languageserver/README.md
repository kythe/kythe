## Building

Be sure to run `npm install` from the `languageserver` directory in order to
fetch dependencies.

We also need to compile the appropriate protos so be sure to run:

```bash
npm run proto
```

This is only required on first setup and when the protos change.

To build the server, run:

```bash
npm run compile
```

or

```bash
npm run watch
```

## Testing

In order to test the server, run:

```bash
npm run test
```

If you wish to run a watch build on tests, run:

```bash
npm run watch_test
```

## Running
The server communicates over the
(Language Server Protocol](https://github.com/Microsoft/language-server-protocol)
(specifically [v2](https://github.com/Microsoft/language-server-protocol/blob/master/versions/protocol-2-x.md)).

The server, by default, makes requests to localhost:8080 which should be running an HTTP interface for the Kythe xref service.

A `.kythe-settings.json` file is required in the root of your project. See Configuration for more details.


#### STDIO communication
```
node dist/src/bin/kythe-languageserver.js --stdio
```

#### Socket communication
```
node dist/src/bin/kythe-languageserver.js --pipe=/tmp/socket.sock
```

#### Node IPC
```
node dist/src/bin/kythe-languageserver.js --ipc
```

## Configuration
The server looks for a `kythe-settings.json` file in the root of the workspace. See this example config file containing all possible options:

```
{
    "mappings": [{
        "local": ":file*",
        "vname": {
            "path": "kythe.io/:file*",
            "corpus": "kythe"
        }
    }],

    "xrefs": {
        "host": "localhost",
        "port": 8080
    }
}
```

## Editor specific instructions
#### Neovim
If you're using [LanguageClient-neovim](https://github.com/autozimu/LanguageClient-neovim), it sets its workspace root as the directory containing the first file opened with the proper extension. The most expeditious workaround is to just open a file at the top level first then open anything else. There is an open issue about this: <https://github.com/autozimu/LanguageClient-neovim/issues/70>
