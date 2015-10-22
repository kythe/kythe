import ts = require('typescript')
import tsu = require('./tsutilities')

export function getDeclarationForName(n : ts.Node) : ts.Declaration {
  switch (n.kind) {
    case ts.SyntaxKind.ConstructorKeyword:
        return <ts.Declaration>n.parent;
    case ts.SyntaxKind.Identifier:
    case ts.SyntaxKind.NumericLiteral:
    case ts.SyntaxKind.StringLiteral:
      if (n.parent && tsu.isDeclaration(n.parent) &&
          (<ts.Declaration>n.parent).name === n) {
        return <ts.Declaration>n.parent;
      }
    default:
      return undefined;
  }
}

export function getQualifiedName(decl : ts.Declaration): string {
  // TODO: should be revised when TS have local types
  let curr : ts.Node = decl;
  let name = "";
  while (curr) {
    switch (curr.kind) {
      case ts.SyntaxKind.Constructor:
          if (curr !== decl) {
              return "constructor"
          }
          else {
              name = "constructor";
          }
          curr = curr.parent;
          break;
      case ts.SyntaxKind.Parameter:
      case ts.SyntaxKind.TypeParameter:
      case ts.SyntaxKind.VariableDeclaration:
      case ts.SyntaxKind.FunctionDeclaration:
        // take a shortcut
        return tsu.declarationNameToString((<ts.Declaration>decl).name);
      case ts.SyntaxKind.GetAccessor:
      case ts.SyntaxKind.SetAccessor:
      case ts.SyntaxKind.PropertyDeclaration:
      case ts.SyntaxKind.PropertySignature:
      case ts.SyntaxKind.PropertyAssignment:
        if (curr.parent && curr.parent.kind === ts.SyntaxKind.TypeLiteral) {
            // take a shortcut
            return tsu.declarationNameToString((<ts.Declaration>decl).name);
        }
      case ts.SyntaxKind.EnumMember:
      case ts.SyntaxKind.ClassDeclaration:
      case ts.SyntaxKind.InterfaceDeclaration:
      case ts.SyntaxKind.EnumDeclaration:
      case ts.SyntaxKind.ModuleDeclaration:
      case ts.SyntaxKind.ImportDeclaration:
        let currName =
            tsu.declarationNameToString((<ts.Declaration>curr).name);
        name = name.length ? currName + "." + name : currName;
      default:
        curr = curr.parent;
    }
  }
  return name;
}
