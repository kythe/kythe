package main

import (
	"context"
	"flag"
	"fmt"
	"go/ast"
	"go/types"
	"log"
	"os"
	"runtime"
	"strconv"
	"sync"

	"kythe.io/kythe/go/indexer/indexer"
	"kythe.io/kythe/go/platform/delimited"
	"kythe.io/kythe/go/platform/indexpack"
	"kythe.io/kythe/go/util/schema"
	"kythe.io/kythe/go/util/schema/edges"
	"kythe.io/kythe/go/util/schema/facts"
	"kythe.io/kythe/go/util/schema/nodes"
	apb "kythe.io/kythe/proto/analysis_proto"
	spb "kythe.io/kythe/proto/storage_proto"
)

var maxProcs = runtime.NumCPU()

//var maxProcs = 1

func main() {
	ctx := context.Background()
	flag.Parse()

	pack, err := indexpack.Open(ctx, os.Args[1], indexpack.UnitType((*apb.CompilationUnit)(nil)))
	if err != nil {
		log.Fatalf("err: %v", err)
	}

	work := make(chan *apb.CompilationUnit)
	out := make(chan *spb.Entry, 1000)

	worker := &worker{
		pack: pack,
		work: work,
		out:  out,
	}

	var wg sync.WaitGroup
	for i := 0; i < maxProcs; i++ {
		wg.Add(1)
		go worker.run(ctx, &wg)
	}

	ifile, err := os.OpenFile("out.raw", os.O_CREATE|os.O_TRUNC|os.O_RDWR, 0777)
	if err != nil {
		log.Fatalf("err: %v", err)
	}
	w := delimited.NewWriter(ifile)

	wg.Add(1)
	go func() {
		defer wg.Done()
		for {
			e, ok := <-out
			if !ok {
				return
			}
			w.PutProto(e)
		}
	}()

	if err := pack.ReadUnits(ctx, "kythe", func(digest string, unit interface{}) error {
		c, ok := unit.(*apb.CompilationUnit)
		if !ok {
			return fmt.Errorf("expected unit of type apb.CompilationUnit")
		}
		if len(c.SourceFile) > 0 {
			work <- c
		}

		return nil
	}); err != nil {
		log.Fatalf("err: %v", err)
	}
	close(work)
	wg.Wait()
	log.Printf("done")
}

type worker struct {
	pack *indexpack.Archive
	work <-chan *apb.CompilationUnit
	out  chan<- *spb.Entry
}

func (w *worker) run(ctx context.Context, wg *sync.WaitGroup) {
	defer wg.Done()
	for {
		unit, ok := <-w.work
		if !ok {
			return
		}

		pkg, err := indexer.Resolve(unit, w.pack.Fetcher(ctx), indexer.AllTypeInfo())
		if err != nil {
			log.Fatalf("err: %v", err)
		}

		for i, o := range pkg.Info.Defs {
			var n schema.Node
			if o == nil {
				continue
			}
			switch obj := o.(type) {
			case *types.Const, *types.Var, *types.PkgName:
				n = schema.Node{
					VName: pkg.VName(o),
					Kind:  nodes.Variable,
				}
			case *types.Func:
				n = schema.Node{
					VName: pkg.VName(o),
					Kind:  nodes.Function,
				}
			case *types.TypeName:
				n = schema.Node{
					VName: pkg.VName(o),
					Kind:  nodes.Record,
				}
			case *types.Label:
				continue
			case *types.Builtin, *types.Nil:
				panic(fmt.Sprintf("writeObject(%T)", obj))
			default:
				panic(fmt.Sprintf("writeObject(%T)", obj))
			}
			n.AddFact(facts.Complete, "definition")
			w.Writes(&n)

			a, childOf := newAnchor(i, pkg.Signature(o)+"#definition", pkg, unit)
			w.Writes(a)
			w.Write(childOf)

			e := schema.Edge{
				Source: a.VName,
				Target: n.VName,
				Kind:   edges.DefinesBinding,
			}
			w.Write(&e)

			if i.Obj == nil || i.Obj.Decl == nil {
				continue
			}
			var docg, commentg *ast.CommentGroup
			switch decl := i.Obj.Decl.(type) {
			case *ast.Field:
				docg = decl.Doc
				commentg = decl.Comment
			case *ast.TypeSpec:
				docg = decl.Doc
				commentg = decl.Comment
			case *ast.FuncDecl:
				docg = decl.Doc
			case *ast.ValueSpec:
				docg = decl.Doc
				commentg = decl.Comment
			case *ast.AssignStmt:
				continue
			default:
				log.Fatalf("unknown decl")
			}

			for _, cg := range []*ast.CommentGroup{docg, commentg} {
				if cg == nil || len(cg.List) == 0 {
					continue
				}

				doc := schema.Node{
					VName: pkg.VName(o),
					Kind:  nodes.Doc,
				}
				doc.AddFact(facts.Text, cg.Text())
				doc.VName.Signature = pkg.Signature(o) + "#doc-" + strconv.Itoa(int(cg.Pos()))

				w.Write(&schema.Edge{
					Source: doc.VName,
					Target: pkg.VName(o),
					Kind:   edges.Documents,
				})

				doca, e := newAnchor(cg, pkg.Signature(o)+"#doca-"+strconv.Itoa(int(cg.Pos())), pkg, unit)
				w.Writes(doca)
				w.Write(e)

				w.Write(&schema.Edge{
					Source: doca.VName,
					Target: pkg.VName(o),
					Kind:   edges.Documents,
				})
			}

		}

		for i, o := range pkg.Info.Uses {
			a, childOf := newAnchor(i, pkg.Signature(o)+"#use-"+strconv.Itoa(int(i.Pos())), pkg, unit)
			w.Writes(a)
			w.Write(childOf)

			e := schema.Edge{
				Source: a.VName,
				Target: pkg.VName(o),
				Kind:   edges.Ref,
			}
			w.Write(&e)
		}

		for p, c := range pkg.SourceText {
			a := schema.Node{
				VName: &spb.VName{
					Path:   p,
					Corpus: unit.VName.Corpus,
				},
				Kind: nodes.File,
			}
			a.AddFact(facts.Text, c)
			a.AddFact(facts.TextEncoding, facts.DefaultTextEncoding)
			w.Writes(&a)
		}

		log.Printf("done indexing %q", unit.VName.Signature)
	}
}

type Enterable interface {
	ToEntry() *spb.Entry
}

type Enterables interface {
	ToEntries() []*spb.Entry
}

func (w *worker) Writes(es Enterables) {
	for _, e := range es.ToEntries() {
		w.out <- e
	}
}

func (w *worker) Write(e Enterable) {
	w.out <- e.ToEntry()
}

func newAnchor(i ast.Node, sig string, pkg *indexer.PackageInfo, unit *apb.CompilationUnit) (*schema.Node, *schema.Edge) {
	fpath := pkg.FileSet.File(i.Pos()).Name()
	a := schema.Node{
		VName: &spb.VName{
			Signature: sig,
			Corpus:    unit.VName.Corpus,
			Path:      fpath,
		},
		Kind: nodes.Anchor,
	}
	start, end := pkg.Span(i)
	a.AddFact(facts.AnchorStart, strconv.Itoa(start))
	a.AddFact(facts.AnchorEnd, strconv.Itoa(end))

	e := schema.Edge{
		Source: a.VName,
		Target: &spb.VName{
			Path:   fpath,
			Corpus: unit.VName.Corpus,
		},
		Kind: edges.ChildOf,
	}
	return &a, &e
}
