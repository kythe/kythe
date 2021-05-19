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
	"errors"
	"flag"
	"fmt"
	"strings"

	ipb "kythe.io/kythe/proto/identifier_go_proto"
)

type identCommand struct {
	baseKytheCommand
	corpora, languages string
}

func (identCommand) Name() string     { return "identifier" }
func (identCommand) Synopsis() string { return "list tickets associated with a given identifier" }
func (identCommand) Usage() string    { return "<identifier>" }
func (c *identCommand) SetFlags(flag *flag.FlagSet) {
	flag.StringVar(&c.corpora, "corpora", "", "Comma-separated list of corpora with which to restrict matches")
	flag.StringVar(&c.languages, "languages", "", "Comma-separated list of languages with which to restrict matches")
}
func (c identCommand) Run(ctx context.Context, flag *flag.FlagSet, api API) error {
	if flag.NArg() == 0 {
		return errors.New("identifier missing")
	} else if flag.NArg() > 1 {
		return fmt.Errorf("only 1 identifier may be given; found: %v", flag.Args())
	}

	req := &ipb.FindRequest{
		Identifier: flag.Arg(0),
	}
	if c.corpora != "" {
		req.Corpus = strings.Split(c.corpora, ",")
	}
	if c.languages != "" {
		req.Languages = strings.Split(c.languages, ",")
	}

	LogRequest(req)
	reply, err := api.IdentifierService.Find(ctx, req)
	if err != nil {
		return err
	}

	return c.displayMatches(reply)
}

func (c identCommand) displayMatches(reply *ipb.FindReply) error {
	if DisplayJSON {
		return PrintJSONMessage(reply)
	}

	for _, m := range reply.Matches {
		kind := m.NodeKind
		if m.NodeSubkind != "" {
			kind += "/" + m.NodeSubkind
		}
		fmt.Printf("%s [kind: %s]\n", m.Ticket, kind)
	}
	return nil
}
