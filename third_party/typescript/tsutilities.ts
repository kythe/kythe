import ts = require('typescript')

// from src/compiler/core.ts
// of https://github.com/Microsoft/TypeScript
// version 07931a3a57293c8f690c95d77961bc794bcad3c9
/**
 * Performs a binary search, finding the index at which 'value' occurs in 'array'.
 * If no such index is found, returns the 2's-complement of first index at which
 * number[index] exceeds number.
 * @param array A sorted array whose first element must be no larger than number
 * @param number The value to be searched for in the array.
 */
export function binarySearch(array: number[], value: number): number {
    let low = 0;
    let high = array.length - 1;

    while (low <= high) {
        let middle = low + ((high - low) >> 1);
        let midValue = array[middle];

        if (midValue === value) {
            return middle;
        }
        else if (midValue > value) {
            high = middle - 1;
        }
        else {
            low = middle + 1;
        }
    }

    return ~low;
}

// from src/services/utilities.ts
// of https://github.com/Microsoft/TypeScript
// version cbe2f3df64d6ceb3a6d928094e6ecb436010ca40

function nodeHasTokens(n: ts.Node): boolean {
    // If we have a token or node that has a non-zero width, it must have tokens.
    // Note, that getWidth() does not take trivia into account.
    return n.getWidth() !== 0;
}

export function isToken(n: ts.Node): boolean {
    return n.kind >= ts.SyntaxKind.FirstToken && n.kind <= ts.SyntaxKind.LastToken;
}

/** Returns the token if position is in [start, end) or if position === end and includeItemAtEndPosition(token) === true */
export function getTouchingToken(sourceFile: ts.SourceFile, position: number, includeItemAtEndPosition?: (n: ts.Node) => boolean) : ts.Node {
    return getTokenAtPositionWorker(sourceFile, position, /*allowPositionInLeadingTrivia*/ false, includeItemAtEndPosition);
}

/** Returns a token if position is in [start-of-leading-trivia, end) */
export function getTokenAtPosition(sourceFile: ts.SourceFile, position: number): ts.Node {
    return getTokenAtPositionWorker(sourceFile, position, /*allowPositionInLeadingTrivia*/ true, /*includeItemAtEndPosition*/ undefined);
}

/** Get the token whose text contains the position */
function getTokenAtPositionWorker(sourceFile: ts.SourceFile, position: number, allowPositionInLeadingTrivia: boolean, includeItemAtEndPosition: (n: ts.Node) => boolean) : ts.Node {
    let current: ts.Node = sourceFile;
    outer: while (true) {
        if (isToken(current)) {
            // exit early
            return current;
        }

        // find the child that contains 'position'
        for (let i = 0, n = current.getChildCount(sourceFile); i < n; i++) {
            let child = current.getChildAt(i);
            let start = allowPositionInLeadingTrivia ? child.getFullStart() : child.getStart(sourceFile);
            if (start <= position) {
                let end = child.getEnd();
                if (position < end || (position === end && child.kind === ts.SyntaxKind.EndOfFileToken)) {
                    current = child;
                    continue outer;
                }
                else if (includeItemAtEndPosition && end === position) {
                    let previousToken = findPrecedingToken(position, sourceFile, child);
                    if (previousToken && includeItemAtEndPosition(previousToken)) {
                        return previousToken;
                    }
                }
            }
        }
        return current;
    }
}

export function findPrecedingToken(position: number, sourceFile: ts.SourceFile, startNode?: ts.Node): ts.Node {
    return find(startNode || sourceFile);

    function findRightmostToken(n: ts.Node): ts.Node {
        if (isToken(n) || n.kind === ts.SyntaxKind.JsxText) {
            return n;
        }

        let children = n.getChildren();
        let candidate = findRightmostChildNodeWithTokens(children, /*exclusiveStartPosition*/ children.length);
        return candidate && findRightmostToken(candidate);

    }

    function find(n: ts.Node): ts.Node {
        if (isToken(n) || n.kind === ts.SyntaxKind.JsxText) {
            return n;
        }

        const children = n.getChildren();
        for (let i = 0, len = children.length; i < len; i++) {
            let child = children[i];
            // condition 'position < child.end' checks if child node end after the position
            // in the example below this condition will be false for 'aaaa' and 'bbbb' and true for 'ccc'
            // aaaa___bbbb___$__ccc
            // after we found child node with end after the position we check if start of the node is after the position.
            // if yes - then position is in the trivia and we need to look into the previous child to find the token in question.
            // if no - position is in the node itself so we should recurse in it.
            // NOTE: JsxText is a weird kind of node that can contain only whitespaces (since they are not counted as trivia).
            // if this is the case - then we should assume that token in question is located in previous child.
            if (position < child.end && (nodeHasTokens(child) || child.kind === ts.SyntaxKind.JsxText)) {
                const start = child.getStart(sourceFile);
                const lookInPreviousChild =
                    (start >= position) || // cursor in the leading trivia
                    (child.kind === ts.SyntaxKind.JsxText && start === child.end); // whitespace only JsxText 
                
                if (lookInPreviousChild) {
                    // actual start of the node is past the position - previous token should be at the end of previous child
                    let candidate = findRightmostChildNodeWithTokens(children, /*exclusiveStartPosition*/ i);
                    return candidate && findRightmostToken(candidate)
                }
                else {
                    // candidate should be in this node
                    return find(child);
                }
            }
        }

        // Debug.assert(startNode !== undefined || n.kind === ts.SyntaxKind.SourceFile);

        // Here we know that none of child token nodes embrace the position, 
        // the only known case is when position is at the end of the file.
        // Try to find the rightmost token in the file without filtering.
        // Namely we are skipping the check: 'position < node.end'
        if (children.length) {
            let candidate = findRightmostChildNodeWithTokens(children, /*exclusiveStartPosition*/ children.length);
            return candidate && findRightmostToken(candidate);
        }
    }

    /// finds last node that is considered as candidate for search (isCandidate(node) === true) starting from 'exclusiveStartPosition'
    function findRightmostChildNodeWithTokens(children: ts.Node[], exclusiveStartPosition: number): ts.Node {
        for (let i = exclusiveStartPosition - 1; i >= 0; --i) {
            if (nodeHasTokens(children[i])) {
                return children[i];
            }
        }
    }
}

// from src/compiler/types.ts
// of https://github.com/Microsoft/TypeScript
// version cbe2f3df64d6ceb3a6d928094e6ecb436010ca40
export const enum CharacterCodes {
  nullCharacter = 0,
  maxAsciiCharacter = 0x7F,

  lineFeed = 0x0A,              // \n
  carriageReturn = 0x0D,        // \r
  lineSeparator = 0x2028,
  paragraphSeparator = 0x2029,
  nextLine = 0x0085,

  // Unicode 3.0 space characters
  space = 0x0020,   // " "
  nonBreakingSpace = 0x00A0,   //
  enQuad = 0x2000,
  emQuad = 0x2001,
  enSpace = 0x2002,
  emSpace = 0x2003,
  threePerEmSpace = 0x2004,
  fourPerEmSpace = 0x2005,
  sixPerEmSpace = 0x2006,
  figureSpace = 0x2007,
  punctuationSpace = 0x2008,
  thinSpace = 0x2009,
  hairSpace = 0x200A,
  zeroWidthSpace = 0x200B,
  narrowNoBreakSpace = 0x202F,
  ideographicSpace = 0x3000,
  mathematicalSpace = 0x205F,
  ogham = 0x1680,

  _ = 0x5F,
  $ = 0x24,

  _0 = 0x30,
  _1 = 0x31,
  _2 = 0x32,
  _3 = 0x33,
  _4 = 0x34,
  _5 = 0x35,
  _6 = 0x36,
  _7 = 0x37,
  _8 = 0x38,
  _9 = 0x39,

  a = 0x61,
  b = 0x62,
  c = 0x63,
  d = 0x64,
  e = 0x65,
  f = 0x66,
  g = 0x67,
  h = 0x68,
  i = 0x69,
  j = 0x6A,
  k = 0x6B,
  l = 0x6C,
  m = 0x6D,
  n = 0x6E,
  o = 0x6F,
  p = 0x70,
  q = 0x71,
  r = 0x72,
  s = 0x73,
  t = 0x74,
  u = 0x75,
  v = 0x76,
  w = 0x77,
  x = 0x78,
  y = 0x79,
  z = 0x7A,

  A = 0x41,
  B = 0x42,
  C = 0x43,
  D = 0x44,
  E = 0x45,
  F = 0x46,
  G = 0x47,
  H = 0x48,
  I = 0x49,
  J = 0x4A,
  K = 0x4B,
  L = 0x4C,
  M = 0x4D,
  N = 0x4E,
  O = 0x4F,
  P = 0x50,
  Q = 0x51,
  R = 0x52,
  S = 0x53,
  T = 0x54,
  U = 0x55,
  V = 0x56,
  W = 0x57,
  X = 0x58,
  Y = 0x59,
  Z = 0x5a,

  ampersand = 0x26,             // &
  asterisk = 0x2A,              // *
  at = 0x40,                    // @
  backslash = 0x5C,             // \
  backtick = 0x60,              // `
  bar = 0x7C,                   // |
  caret = 0x5E,                 // ^
  closeBrace = 0x7D,            // }
  closeBracket = 0x5D,          // ]
  closeParen = 0x29,            // )
  colon = 0x3A,                 // :
  comma = 0x2C,                 // ,
  dot = 0x2E,                   // .
  doubleQuote = 0x22,           // "
  equals = 0x3D,                // =
  exclamation = 0x21,           // !
  greaterThan = 0x3E,           // >
  hash = 0x23,                  // #
  lessThan = 0x3C,              // <
  minus = 0x2D,                 // -
  openBrace = 0x7B,             // {
  openBracket = 0x5B,           // [
  openParen = 0x28,             // (
  percent = 0x25,               // %
  plus = 0x2B,                  // +
  question = 0x3F,              // ?
  semicolon = 0x3B,             // ;
  singleQuote = 0x27,           // '
  slash = 0x2F,                 // /
  tilde = 0x7E,                 // ~

  backspace = 0x08,             // \b
  formFeed = 0x0C,              // \f
  byteOrderMark = 0xFEFF,
  tab = 0x09,                   // \t
  verticalTab = 0x0B,           // \v
}


// from src/compiler/scanner.ts
// of https://github.com/Microsoft/TypeScript
// version cbe2f3df64d6ceb3a6d928094e6ecb436010ca40

export function isLineBreak(ch: number): boolean {
    // ES5 7.3:
    // The ECMAScript line terminator characters are listed in Table 3.
    //     Table 3: Line Terminator Characters
    //     Code Unit Value     Name                    Formal Name
    //     \u000A              Line Feed               <LF>
    //     \u000D              Carriage Return         <CR>
    //     \u2028              Line separator          <LS>
    //     \u2029              Paragraph separator     <PS>
    // Only the characters in Table 3 are treated as line terminators. Other new line or line
    // breaking characters are treated as white space but not as line terminators.

    return ch === CharacterCodes.lineFeed ||
        ch === CharacterCodes.carriageReturn ||
        ch === CharacterCodes.lineSeparator ||
        ch === CharacterCodes.paragraphSeparator;
}

export function isWhiteSpace(ch: number): boolean {
    // Note: nextLine is in the Zs space, and should be considered to be a whitespace.
    // It is explicitly not a line-break as it isn't in the exact set specified by EcmaScript.
    return ch === CharacterCodes.space ||
        ch === CharacterCodes.tab ||
        ch === CharacterCodes.verticalTab ||
        ch === CharacterCodes.formFeed ||
        ch === CharacterCodes.nonBreakingSpace ||
        ch === CharacterCodes.nextLine ||
        ch === CharacterCodes.ogham ||
        ch >= CharacterCodes.enQuad && ch <= CharacterCodes.zeroWidthSpace ||
        ch === CharacterCodes.narrowNoBreakSpace ||
        ch === CharacterCodes.mathematicalSpace ||
        ch === CharacterCodes.ideographicSpace ||
        ch === CharacterCodes.byteOrderMark;
  }

const shebangTriviaRegex = /^#!.*/;

function isShebangTrivia(text: string, pos: number) {
    // Shebangs check must only be done at the start of the file
    // Debug.assert(pos === 0);
    return shebangTriviaRegex.test(text);
}

function scanShebangTrivia(text: string, pos: number) {
    let shebang = shebangTriviaRegex.exec(text)[0];
    pos = pos + shebang.length;
    return pos;
}

let mergeConflictMarkerLength = "<<<<<<<".length;

function scanConflictMarkerTrivia(text: string, pos: number, error?: ts.ErrorCallback) {
    /*if (error) {
        error(Diagnostics.Merge_conflict_marker_encountered, mergeConflictMarkerLength);
    }*/

    let ch = text.charCodeAt(pos);
    let len = text.length;

    if (ch === CharacterCodes.lessThan || ch === CharacterCodes.greaterThan) {
        while (pos < len && !isLineBreak(text.charCodeAt(pos))) {
            pos++;
        }
    }
    else {
        // Debug.assert(ch === CharacterCodes.equals);
        // Consume everything from the start of the mid-conlict marker to the start of the next
        // end-conflict marker.
        while (pos < len) {
            let ch = text.charCodeAt(pos);
            if (ch === CharacterCodes.greaterThan && isConflictMarkerTrivia(text, pos)) {
                break;
            }

            pos++;
        }
    }

    return pos;
}

function isConflictMarkerTrivia(text: string, pos: number) {
    // Debug.assert(pos >= 0);

    // Conflict markers must be at the start of a line.
    if (pos === 0 || isLineBreak(text.charCodeAt(pos - 1))) {
        let ch = text.charCodeAt(pos);

        if ((pos + mergeConflictMarkerLength) < text.length) {
            for (let i = 0, n = mergeConflictMarkerLength; i < n; i++) {
                if (text.charCodeAt(pos + i) !== ch) {
                    return false;
                }
            }

            return ch === CharacterCodes.equals ||
                text.charCodeAt(pos + mergeConflictMarkerLength) === CharacterCodes.space;
        }
    }

    return false;
}

export function skipTrivia(text: string, pos: number, stopAfterLineBreak?: boolean): number {
  // Keep in sync with couldStartTrivia
  while (true) {
      let ch = text.charCodeAt(pos);
      switch (ch) {
          case CharacterCodes.carriageReturn:
              if (text.charCodeAt(pos + 1) === CharacterCodes.lineFeed) {
                  pos++;
              }
          case CharacterCodes.lineFeed:
              pos++;
              if (stopAfterLineBreak) {
                  return pos;
              }
              continue;
          case CharacterCodes.tab:
          case CharacterCodes.verticalTab:
          case CharacterCodes.formFeed:
          case CharacterCodes.space:
              pos++;
              continue;
          case CharacterCodes.slash:
              if (text.charCodeAt(pos + 1) === CharacterCodes.slash) {
                  pos += 2;
                  while (pos < text.length) {
                      if (isLineBreak(text.charCodeAt(pos))) {
                          break;
                      }
                      pos++;
                  }
                  continue;
              }
              if (text.charCodeAt(pos + 1) === CharacterCodes.asterisk) {
                  pos += 2;
                  while (pos < text.length) {
                      if (text.charCodeAt(pos) === CharacterCodes.asterisk && text.charCodeAt(pos + 1) === CharacterCodes.slash) {
                          pos += 2;
                          break;
                      }
                      pos++;
                  }
                  continue;
              }
              break;

          case CharacterCodes.lessThan:
          case CharacterCodes.equals:
          case CharacterCodes.greaterThan:
              if (isConflictMarkerTrivia(text, pos)) {
                  pos = scanConflictMarkerTrivia(text, pos);
                  continue;
              }
              break;

          case CharacterCodes.hash:
              if (pos === 0 && isShebangTrivia(text, pos)) {
                  pos = scanShebangTrivia(text, pos);
                  continue;
              }
              break;

          default:
              if (ch > CharacterCodes.maxAsciiCharacter && (isWhiteSpace(ch) || isLineBreak(ch))) {
                  pos++;
                  continue;
              }
              break;
      }
      return pos;
  }
}

// from src/compiler/utilities.ts
// of https://github.com/Microsoft/TypeScript
// version cbe2f3df64d6ceb3a6d928094e6ecb436010ca40

export function isDeclaration(node: ts.Node): boolean {
    switch (node.kind) {
        case ts.SyntaxKind.ArrowFunction:
        case ts.SyntaxKind.BindingElement:
        case ts.SyntaxKind.ClassDeclaration:
        case ts.SyntaxKind.ClassExpression:
        case ts.SyntaxKind.Constructor:
        case ts.SyntaxKind.EnumDeclaration:
        case ts.SyntaxKind.EnumMember:
        case ts.SyntaxKind.ExportSpecifier:
        case ts.SyntaxKind.FunctionDeclaration:
        case ts.SyntaxKind.FunctionExpression:
        case ts.SyntaxKind.GetAccessor:
        case ts.SyntaxKind.ImportClause:
        case ts.SyntaxKind.ImportEqualsDeclaration:
        case ts.SyntaxKind.ImportSpecifier:
        case ts.SyntaxKind.InterfaceDeclaration:
        case ts.SyntaxKind.MethodDeclaration:
        case ts.SyntaxKind.MethodSignature:
        case ts.SyntaxKind.ModuleDeclaration:
        case ts.SyntaxKind.NamespaceImport:
        case ts.SyntaxKind.Parameter:
        case ts.SyntaxKind.PropertyAssignment:
        case ts.SyntaxKind.PropertyDeclaration:
        case ts.SyntaxKind.PropertySignature:
        case ts.SyntaxKind.SetAccessor:
        case ts.SyntaxKind.ShorthandPropertyAssignment:
        case ts.SyntaxKind.TypeAliasDeclaration:
        case ts.SyntaxKind.TypeParameter:
        case ts.SyntaxKind.VariableDeclaration:
            return true;
    }
    return false;
}

// Returns true if this node is missing from the actual source code. A 'missing' node is different
// from 'undefined/defined'. When a node is undefined (which can happen for optional nodes
// in the tree), it is definitely missing. However, a node may be defined, but still be
// missing.  This happens whenever the parser knows it needs to parse something, but can't
// get anything in the source code that it expects at that location. For example:
//
//          let a: ;
//
// Here, the Type in the Type-Annotation is not-optional (as there is a colon in the source
// code). So the parser will attempt to parse out a type, and will create an actual node.
// However, this node will be 'missing' in the sense that no actual source-code/tokens are
// contained within it.
export function nodeIsMissing(node: ts.Node) {
  if (!node) {
      return true;
  }

  return node.pos === node.end && node.pos >= 0 && node.kind !== ts.SyntaxKind.EndOfFileToken;
}

export function getSourceFileOfNode(node: ts.Node): ts.SourceFile {
    while (node && node.kind !== ts.SyntaxKind.SourceFile) {
        node = node.parent;
    }
    return <ts.SourceFile>node;
}

export function getSourceTextOfNodeFromSourceFile(sourceFile: ts.SourceFile, node: ts.Node, includeTrivia = false): string {
    if (nodeIsMissing(node)) {
        return "";
    }

    let text = sourceFile.text;
    return text.substring(includeTrivia ? node.pos : skipTrivia(text, node.pos), node.end);
}

export function getFullWidth(node: ts.Node) {
    return node.end - node.pos;
}

export function getTextOfNode(node: ts.Node, includeTrivia = false): string {
    return getSourceTextOfNodeFromSourceFile(getSourceFileOfNode(node), node, includeTrivia);
}

// Return display name of an identifier
// Computed property names will just be emitted as "[<expr>]", where <expr> is the source
// text of the expression in the computed property.
export function declarationNameToString(name: ts.DeclarationName) {
    return getFullWidth(name) === 0 ? "(Missing)" : getTextOfNode(name);
}
