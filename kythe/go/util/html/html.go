/*
 * Copyright 2014 The Kythe Authors. All rights reserved.
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

// Package html is a set of utilities for manipulating html Nodes.
package html // import "kythe.io/kythe/go/util/html"

import (
	"bytes"
	"fmt"

	"kythe.io/kythe/go/util/log"

	"golang.org/x/net/html"
)

// Decoration is a template for an HTML node that should span some textual
// offsets.
type Decoration struct {
	// 0-based character offsets for the span of the decoration.
	Start, End int

	// Template to use as the HTML decoration Node.
	Node *html.Node
}

// Decorate will apply the given slice of Decorations to the HTML tree starting
// at root.
func Decorate(root *html.Node, decor []Decoration) {
	offsets := textualOffsets(root)

	for _, d := range decor {
		nodes := nodeRange(root, offsets, d.Start, d.End)

		decorNode := copyNode(d.Node)
		parent := nodes[0].Parent
		parent.InsertBefore(decorNode, nodes[0])
		for _, n := range nodes {
			parent.RemoveChild(n)
			decorNode.AppendChild(n)
		}
		offsets.update(decorNode, d.Start)
	}
}

// nodeOffsets is a structure storing the 0-based character offsets at the
// beginning and ending of html.Nodes.
type nodeOffsets struct {
	starts map[*html.Node]int
	ends   map[*html.Node]int
}

// Bounds returns the starting and ending offset of n.
func (o *nodeOffsets) Bounds(n *html.Node) (int, int) {
	return o.starts[n], o.ends[n]
}

// textualOffsets returns nodeOffsets for every html.Node in the tree starting
// at root.
func textualOffsets(root *html.Node) *nodeOffsets {
	offsets := &nodeOffsets{make(map[*html.Node]int), make(map[*html.Node]int)}
	offsets.update(root, 0)
	return offsets
}

// update updates the nodeOffsets for every html.Node starting at root, assuming
// offset is the starting offset for the root html.Node.
func (o *nodeOffsets) update(root *html.Node, offset int) int {
	o.starts[root] = offset

	if root.Type == html.TextNode {
		offset += len(root.Data)
	} else {
		for n := root.FirstChild; n != nil; n = n.NextSibling {
			offset = o.update(n, offset)
		}
	}

	o.ends[root] = offset
	return offset
}

// nodeRange returns an ordered slice of sibling html.Nodes that span exactly
// the range between the given start and end offsets. If necessary, nodeRange
// will split html.Nodes so that the returned range is precise.
func nodeRange(root *html.Node, offsets *nodeOffsets, start, end int) []*html.Node {
	if rootStart, rootEnd := offsets.Bounds(root); rootStart > start || rootEnd < end {
		log.Fatalf("nodeRange: root %d%q Node (%d → %d) does not contain range %d → %d",
			root.Type, root.Data, rootStart, rootEnd, start, end)
	}

	var nodes []*html.Node
	for n := root.FirstChild; n != nil; n = n.NextSibling {
		nStart, nEnd := offsets.Bounds(n)
		if nStart < end && nEnd >= start {
			nodes = append(nodes, n)
		} else if nStart > end {
			break
		}
	}

	if len(nodes) == 1 && nodes[0] != root {
		return nodeRange(nodes[0], offsets, start, end)
	} else if len(nodes) == 0 {
		nodes = []*html.Node{root}
	}

	// Slice end nodes, if necessary
	if rangeStart, _ := offsets.Bounds(nodes[0]); start != rangeStart {
		_, m := sliceNode(offsets, nodes[0], start)
		nodes = append([]*html.Node{m}, nodes[1:]...)
	}
	if rangeEnd, _ := offsets.Bounds(nodes[len(nodes)-1]); end != rangeEnd {
		n, _ := sliceNode(offsets, nodes[len(nodes)-1], end)
		nodes = append(nodes[:len(nodes)-1], n)
	}

	return nodes
}

// sliceNode returns the two halves of the HTML tree starting at node after
// splitting it at the given textual offset.
func sliceNode(offsets *nodeOffsets, node *html.Node, offset int) (*html.Node, *html.Node) {
	origStart, origEnd := offsets.Bounds(node)
	if origStart > offset || origEnd < offset {
		log.Fatalf("sliceNode: offset %d out of node's span (%d → %d)", offset, origStart, origEnd)
	}

	n, m := copyNode(node), copyNode(node)
	parent := node.Parent
	if parent != nil {
		parent.InsertBefore(n, node)
		parent.InsertBefore(m, node)
		parent.RemoveChild(node)
	}

	switch node.Type {
	default:
		log.Fatalf("Unhandled node kind: %d", node.Type)
	case html.ElementNode:
		child := node.FirstChild
		for child != nil {
			next := child.NextSibling

			if _, end := offsets.Bounds(child); end <= offset {
				node.RemoveChild(child)
				n.AppendChild(child)
			} else if start, _ := offsets.Bounds(child); start > offset {
				node.RemoveChild(child)
				m.AppendChild(child)
			} else {
				left, right := sliceNode(offsets, child, offset)
				node.RemoveChild(left)
				node.RemoveChild(right)
				n.AppendChild(left)
				m.AppendChild(right)
			}

			child = next
		}
	case html.TextNode:
		mark := offset - origStart
		n.Data = node.Data[:mark]
		m.Data = node.Data[mark:]
	}

	if split := offsets.update(n, origStart); split != offset {
		log.Fatalf("split %d ≠ %d", split, offset)
	}
	if newEnd := offsets.update(m, offset); newEnd != origEnd {
		log.Fatalf("end %d ≠ %d", newEnd, origEnd)
	}

	return n, m
}

// PlainText returns the concatenation of the textual contents for the given
// html Nodes.
func PlainText(nodes ...*html.Node) string {
	var text bytes.Buffer
	for _, n := range nodes {
		switch n.Type {
		default:
			log.Fatalf("PlainText: unhandled node kind: %d", n.Type)
		case html.DocumentNode, html.ElementNode:
			for child := n.FirstChild; child != nil; child = child.NextSibling {
				fmt.Fprint(&text, PlainText(child))
			}
		case html.TextNode:
			fmt.Fprintf(&text, n.Data)
		}
	}
	return text.String()
}

// copyNode returns a shallow copy of n excluding sibling/parent/child pointers.
func copyNode(n *html.Node) *html.Node {
	return &html.Node{
		Type:      n.Type,
		Data:      n.Data,
		DataAtom:  n.DataAtom,
		Namespace: n.Namespace,
		Attr:      n.Attr,
	}
}

// Zip returns the Node at the end of the specified path where path contains
// only the following characters:
//
//	'u' Parent
//	'f' FirstChild
//	'l' LastChild
//	'n' NextSibling
//	'p' PrevSibling
func Zip(n *html.Node, path string) (*html.Node, error) {
	for i, step := range path {
		if n == nil {
			return nil, fmt.Errorf("ran into nil Node after %d steps: %q", i, path)
		}
		switch step {
		case 'f':
			n = n.FirstChild
		case 'l':
			n = n.LastChild
		case 'n':
			n = n.NextSibling
		case 'p':
			n = n.PrevSibling
		case 'u':
			n = n.Parent
		default:
			return nil, fmt.Errorf("invalid zip path (%q) rune: %q", path, step)
		}
	}
	return n, nil
}

// MustZip delegates to Zip and panics on any error.
func MustZip(n *html.Node, path string) *html.Node {
	res, err := Zip(n, path)
	if err != nil {
		panic(err)
	}
	return res
}
