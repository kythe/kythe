// Deps:
//   :vnames
//   rust_analyzer

use rust_analyzer::{
    hir::{Crate, Semantics},
    ide::RootDatabase,
    ide_db::defs::Definition,
};
use std::collections::HashSet;
use vnames::VNameFactory;

pub struct IndexingContext<'a> {
    pub db: &'a RootDatabase,
    pub sema: &'a Semantics<'a, RootDatabase>,
    pub vnames: &'a VNameFactory<'a>,
    /// We should only index members of the current crate.
    pub krate: Crate,
    queue: Vec<Definition>,
    seen: HashSet<Definition>,
}

impl<'a> IndexingContext<'a> {
    pub fn new(
        krate: Crate,
        sema: &'a Semantics<'a, RootDatabase>,
        vnames: &'a VNameFactory,
    ) -> IndexingContext<'a> {
        IndexingContext {
            db: sema.db,
            sema,
            vnames,
            krate,
            queue: Vec::new(),
            seen: HashSet::new(),
        }
    }

    pub fn enqueue(&mut self, node: Definition) {
        if node.krate(self.db) == Some(self.krate) && self.seen.insert(node) {
            self.queue.push(node);
        }
    }

    pub fn enqueue_all(&mut self, nodes: impl Iterator<Item = Definition>) {
        nodes.for_each(|node| self.enqueue(node))
    }

    pub fn pop_queue(&mut self) -> Option<Definition> {
        self.queue.pop()
    }
}
