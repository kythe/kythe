/*
 * Copyright 2014 Google Inc. All rights reserved.
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

package html

import (
	"bytes"
	"testing"

	"golang.org/x/net/html"
)

const testHTML = "<b>test</b> <span>body <i>text</i></span>"

func TestSliceNode(t *testing.T) {
	root := parseHTML(t, testHTML)
	fullPlain := PlainText(root)
	o := textualOffsets(root)

	tests := []struct {
		path              string
		start, split, end int
	}{
		{"flf", 0, 2, 4}, // <b>
		{"flfn", 2, 3, 4},
		{"flfnnnn", 5, 7, 14}, // <span>
		{"flfnnnnf", 5, 6, 7},
		{"flfnnnn", 5, 6, 7},
		{"flfnnnnnn", 7, 12, 14},
	}

	for _, test := range tests {
		node := MustZip(root, test.path)
		plain := PlainText(node)
		left, right := sliceNode(o, node, test.split)
		checkBounds(t, o, left, test.start, test.split)
		checkBounds(t, o, right, test.split, test.end)

		if resultPlain := PlainText(left, right); plain != resultPlain {
			t.Errorf("text: Expected %q; Found %q", plain, resultPlain)
		}
		if leftPlain, expected := PlainText(left), fullPlain[test.start:test.split]; leftPlain != expected {
			t.Errorf("left text: Expected %q; Found %q", leftPlain, expected)
		}
		if rightPlain, expected := PlainText(right), fullPlain[test.split:test.end]; rightPlain != expected {
			t.Errorf("right text: Expected %q; Found %q", rightPlain, expected)
		}

		if res := MustZip(root, test.path); left != res {
			t.Errorf("left: Expected %v; Found %v", left, res)
		}
		if res := MustZip(root, test.path+"n"); right != res {
			t.Errorf("right: Expected %v; Found %v", right, res)
		}
	}
}

func TestOffsets(t *testing.T) {
	n := parseHTML(t, testHTML)
	o := textualOffsets(n)

	tests := []struct {
		path       string
		start, end int
	}{
		{"", 0, 14},
		{"f", 0, 14},  // <html>
		{"ff", 0, 0},  // <title>
		{"fl", 0, 14}, // <body>
		{"flf", 0, 4}, // <b>
		{"flff", 0, 4},
		{"flfn", 4, 5},
		{"flfnn", 5, 14}, // <span>
		{"flfnnf", 5, 10},
		{"flfnnfn", 10, 14}, // <i>
		{"flfnnfnf", 10, 14},
	}

	for _, test := range tests {
		checkBounds(t, o, MustZip(n, test.path), test.start, test.end)
	}
}

func checkBounds(t *testing.T, offsets *nodeOffsets, n *html.Node, start, end int) {
	if s, e := offsets.Bounds(n); s != start {
		t.Errorf("checkBounds: start expected %d; received %d", start, s)
	} else if e != end {
		t.Errorf("checkBounds: end expected %d; received %d", end, e)
	}
}

func parseHTML(t *testing.T, s string) *html.Node {
	buf := new(bytes.Buffer)
	_, err := buf.Write([]byte(s))
	if err != nil {
		t.Error("Could not write string to Buffer")
	}
	n, err := html.Parse(buf)
	if err != nil {
		t.Error("Could not parse HTML")
	}
	return n
}

func TestZip(t *testing.T) {
	n := parseHTML(t, testHTML)

	tests := []struct {
		expected *html.Node
		path     string
	}{
		{n, ""},
		{n.FirstChild, "f"},
		{n.FirstChild.FirstChild.NextSibling, "ffn"},
		{n.FirstChild, "ffnu"},
		{n.Parent, "u"},
		{n.LastChild.LastChild, "ll"},
		{n.LastChild.LastChild.FirstChild.NextSibling, "llfnnp"},
	}

	for _, test := range tests {
		if res, err := Zip(n, test.path); err != nil {
			t.Error(err)
		} else if res != test.expected {
			t.Errorf("Path %q; Expected %v; Received %v", test.path, test.expected, res)
		}
	}
}

func TestPlainText(t *testing.T) {
	root := parseHTML(t, testHTML)

	tests := []struct{ path, expected string }{
		{"", "test body text"},
		{"fl", "test body text"},
		{"flf", "test"},
		{"flff", "test"},
		{"flfn", " "},
		{"flfnn", "body text"},
		{"flfnnf", "body "},
		{"flfnnl", "text"},
		{"flfnnlf", "text"},
	}

	for _, test := range tests {
		if res := PlainText(MustZip(root, test.path)); test.expected != res {
			t.Errorf("Path %q; Expected %q; Found %q", test.path, test.expected, res)
		}
	}
}
