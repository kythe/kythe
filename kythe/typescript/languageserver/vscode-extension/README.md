## Building

Be sure to run `npm install` from the `vscode-extension` directory in order to
fetch dependencies

This extension is built using either:

```bash
npm run compile
```

or

```bash
npm run watch
```

## Debugging

It should be possible to debug the extension by opening the `vscode-extension`
folder in Visual Studio Code and pressing `F5`.

For more information, see `.vscode/launch.json` and 
[Running and Debugging Your Extension](https://code.visualstudio.com/docs/extensions/debugging-extensions)

## Notes

The extension pulls the language server from the local filesystem via a
symlink. This means changes to the server will not be reflected in the
extension unless the server is rebuilt. We recommend running `npm run watch`
in the server directory as well as this one.

