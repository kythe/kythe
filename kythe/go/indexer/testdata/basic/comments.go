// Package comment contains comments.
package comment

//- Pkg.node/kind package
//- Pkg.doc/uri "http://godoc.org/kythe/go/indexer/comment_test"
//- PkgDoc.node/kind doc
//- PkgDoc documents Pkg
//- PkgDoc.text "Package comment contains comments."

//- @+3"foo" defines/binding Foo

/* foo comment */
var foo int

//- Foo.node/kind variable
//- FooDoc.node/kind doc
//- FooDoc.text "foo comment"
//- FooDoc documents Foo

// General constants.
const (
	//- @+2"bar" defines/binding Bar

	bar = 5

	//- @+3"baz" defines/binding Baz

	// baz comment
	baz = "quux" // doc comments are preferred

	//- @+2"ほげ" defines/binding JFoo

	ほげ = 42 // the japanese equivalent of foo
)

// Names without their own comments inherit the block comment, if present.
// This may result in duplicating the comment text to different nodes.
//
//- Bar.node/kind constant
//- BarComment.node/kind doc
//- BarComment.text "General constants."
//- BarComment documents Bar

// Names with their own comments prefer them.
//- Baz.node/kind constant
//- BazComment.node/kind doc
//- BazComment.text "baz comment"
//- BazComment documents Baz

// Line comments are used when there is no above-line "doc" comment.
//- !{ _BazLineComment.text "doc comments are preferred" }
//- LineComment documents JFoo
//- LineComment.node/kind doc
//- LineComment.text "the japanese equivalent of foo"

//- @+3"alpha" defines/binding Alpha

// alpha is a function.
func alpha() {}

//- Alpha.node/kind function
//- AlphaDoc.node/kind doc
//- AlphaDoc.text "alpha is a function."
//- AlphaDoc documents Alpha

//- @+3"widget" defines/binding Widget

// widget comment
type widget struct {
	//- Lawyer.node/kind variable
	//- LawyerDoc documents Lawyer
	//- LawyerDoc.text "Lawyer takes the bar."
	//-
	//- @+3"Lawyer" defines/binding Lawyer

	// Lawyer takes the bar.
	Lawyer bool

	//- ErrField.node/kind variable
	//- ErrDoc documents ErrField
	//- ErrDoc.text "What went wrong."
	//-
	//- @+3"error" defines/binding ErrField

	// What went wrong.
	error

	//- LineDoc documents ByLineField
	//- LineDoc.text "this is a line comment"
	//- @+2ByLine defines/binding ByLineField

	ByLine int // this is a line comment
}

//- Widget.node/kind record
//- WidgetComment.node/kind doc
//- WidgetComment.text "widget comment"
//- WidgetComment documents Widget
