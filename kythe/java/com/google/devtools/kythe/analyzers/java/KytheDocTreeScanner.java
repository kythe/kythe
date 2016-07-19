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

package com.google.devtools.kythe.analyzers.java;

import com.google.devtools.kythe.analyzers.base.EntrySet;
import com.sun.source.doctree.ReferenceTree;
import com.sun.source.util.DocTreeScanner;
import com.sun.tools.javac.code.Symbol;
import com.sun.tools.javac.code.Symtab;
import com.sun.tools.javac.tree.DCTree.DCDocComment;
import com.sun.tools.javac.tree.DCTree.DCReference;
import com.sun.tools.javac.tree.DocCommentTable;
import com.sun.tools.javac.tree.JCTree;
import com.sun.tools.javac.tree.JCTree.JCFieldAccess;
import com.sun.tools.javac.tree.JCTree.JCIdent;
import com.sun.tools.javac.util.Name;
import java.util.ArrayList;
import java.util.LinkedList;
import java.util.List;

public class KytheDocTreeScanner extends DocTreeScanner<Void, DCDocComment> {
  private final KytheTreeScanner treeScanner;
  private final DocCommentTable table;
  private final List<MiniAnchor<Symbol>> miniAnchors;

  public KytheDocTreeScanner(KytheTreeScanner treeScanner, DocCommentTable table) {
    this.treeScanner = treeScanner;
    this.table = table;
    this.miniAnchors = new ArrayList<>();
  }

  public boolean visitDocComment(JCTree tree, EntrySet node) {
    final DCDocComment doc = table.getCommentTree(tree);
    if (doc == null) {
      return false;
    }

    miniAnchors.clear();
    doc.accept(this, doc);
    int startChar = (int) doc.getSourcePosition(doc);

    String bracketed =
        MiniAnchor.bracket(
            doc.comment.getText(),
            new MiniAnchor.PositionTransform() {
              @Override
              public int transform(int pos) {
                return doc.comment.getSourcePos(pos);
              }
            },
            miniAnchors);
    List<Symbol> anchoredTo = new ArrayList<>(miniAnchors.size());
    for (MiniAnchor<Symbol> miniAnchor : miniAnchors) {
      anchoredTo.add(miniAnchor.getAnchoredTo());
    }
    treeScanner.emitDoc(bracketed, anchoredTo, node);
    return treeScanner.emitCommentsOnLine(treeScanner.charToLine(startChar), node);
  }

  @Override
  public Void visitReference(ReferenceTree tree, DCDocComment doc) {
    DCReference ref = (DCReference) tree;

    // TODO(schroederc): handle non-expression references (e.g. "#memberName" or "#methodName()")
    Symbol sym = findSymbol(ref.qualifierExpression);
    if (sym == null) {
      return null;
    }

    int startPos = (int) ref.getSourcePosition(doc);
    int endPos = ref.getEndPos(doc);

    treeScanner.emitDocReference(sym, startPos, endPos);
    miniAnchors.add(new MiniAnchor<Symbol>(sym, startPos, endPos));

    return null;
  }

  private Symbol findSymbol(JCTree tree) {
    Symtab syms = treeScanner.getSymbols();
    Name name;
    if (tree instanceof JCIdent) {
      JCIdent ident = (JCIdent) tree;
      name = ident.name;
    } else if (tree instanceof JCFieldAccess) {
      JCFieldAccess field = (JCFieldAccess) tree;
      name = field.name.table.fromString(field.selected.toString()).append('.', field.name);
    } else {
      return null;
    }

    // TODO(schroederc): handle member references (e.g. "className#memberName")
    if (syms.classes.containsKey(name)) {
      return syms.classes.get(name);
    } else if (!name.toString().matches("[$.#]")) {
      List<Name> matches = new LinkedList<>();
      for (Name clsName : syms.classes.keySet()) {
        String[] parts = clsName.toString().split("[$.]");
        if (parts[parts.length - 1].equals(name.toString())) {
          matches.add(clsName);
        }
      }
      if (matches.size() == 1) {
        return syms.classes.get(matches.get(0));
      }
    }

    return null;
  }
}
