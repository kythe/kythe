/*
 * Copyright 2017 The Kythe Authors. All rights reserved.
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

package cli

import (
	"context"
	"flag"
	"fmt"
	"strings"

	"kythe.io/kythe/go/util/log"
	xpb "kythe.io/kythe/proto/xref_go_proto"
)

type docsCommand struct {
	baseKytheCommand
	nodeFilters     string
	includeChildren bool
}

func (docsCommand) Name() string     { return "docs" }
func (docsCommand) Synopsis() string { return "display documentation for a node" }
func (c *docsCommand) SetFlags(flag *flag.FlagSet) {
	flag.StringVar(&c.nodeFilters, "filters", "", "Comma-separated list of node fact filters (default returns all)")
	flag.BoolVar(&c.includeChildren, "include_children", false, "Include documentation for children of the given node")
}
func (c docsCommand) Run(ctx context.Context, flag *flag.FlagSet, api API) error {
	req := &xpb.DocumentationRequest{
		Ticket:          flag.Args(),
		IncludeChildren: c.includeChildren,
	}
	if c.nodeFilters != "" {
		req.Filter = strings.Split(c.nodeFilters, ",")
	}
	LogRequest(req)
	reply, err := api.XRefService.Documentation(ctx, req)
	if err != nil {
		return err
	}
	return c.displayDocumentation(reply)
}

func findLinkText(rawText string) []string {
	var linkText []string
	var current []int
	var invalid bool
	for i := 0; i < len(rawText); i++ {
		c := rawText[i]
		switch c {
		case '[':
			current = append(current, len(linkText))
			linkText = append(linkText, "")
		case ']':
			if len(current) == 0 {
				invalid = true
				continue
			}
			current = current[:len(current)-1]
		default:
			if c == '\\' {
				if i+1 >= len(rawText) {
					invalid = true
					continue
				}
				i++
				c = rawText[i]
			}
			for _, l := range current {
				linkText[l] += string(c)
			}
		}
	}
	if invalid {
		log.Warningf("invalid document raw text: %q", rawText)
	}
	return linkText
}

func displayDoc(indent string, doc *xpb.DocumentationReply_Document) {
	fmt.Println(indent + showSignature(doc.MarkedSource))
	if len(doc.Text.GetRawText()) > 0 {
		fmt.Println(indent + doc.Text.RawText)
		linkText := findLinkText(doc.Text.RawText)
		for i, link := range doc.Text.Link {
			if i >= len(linkText) {
				log.Warningf("mismatch between raw text and number of links: %v", doc)
				break
			}
			if len(link.Definition) > 0 {
				fmt.Printf("%s    %s: %s\n", indent, linkText[i], strings.Join(link.Definition, "\t"))
			}
		}
	}

	for _, child := range doc.Children {
		fmt.Println()
		displayDoc(indent+"  ", child)
	}
}

func (c docsCommand) displayDocumentation(reply *xpb.DocumentationReply) error {
	if DisplayJSON {
		return PrintJSONMessage(reply)
	} else if len(reply.Document) == 0 {
		return nil
	}

	displayDoc("", reply.Document[0])
	for _, doc := range reply.Document[1:] {
		fmt.Println()
		displayDoc("", doc)
	}
	return nil
}
