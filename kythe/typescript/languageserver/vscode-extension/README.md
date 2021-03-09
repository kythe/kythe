## Building

Be sure to run `npm install` from the `vscode-extension` directory in order to
fetch dependencies.

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

## Packaging

To package a `.vsix` file for deployment, enter the `vscode-extension` directory
and run `vsce package`. This will write a `kythe-X.Y.Z.vsix` file. If you have
made changes be sure to update the version in `package.json`.

If necessary, install `vsce` by running `npm install -g vsce`.
