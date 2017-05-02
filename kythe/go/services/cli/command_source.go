/*
 * Copyright 2017 Google Inc. All rights reserved.
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
	"regexp"
	"strconv"
	"strings"

	cpb "kythe.io/kythe/proto/common_proto"
	xpb "kythe.io/kythe/proto/xref_proto"
)

const spanHelp = `Limit results to this span (e.g. "10-30", "b1462-b1847", "3:5-3:10")
      Formats:
        b\d+-b\d+             -- Byte-offsets
        \d+(:\d+)?-\d+(:\d+)? -- Line offsets with optional column offsets`

var (
	byteOffsetPointRE = regexp.MustCompile(`^b(\d+)$`)
	lineNumberPointRE = regexp.MustCompile(`^(\d+)(:(\d+))?$`)
)

func parsePoint(p string) (*cpb.Point, error) {
	if m := byteOffsetPointRE.FindStringSubmatch(p); m != nil {
		offset, err := strconv.Atoi(m[1])
		if err != nil {
			return nil, fmt.Errorf("invalid byte-offset: %v", err)
		}
		return &cpb.Point{ByteOffset: int32(offset)}, nil
	} else if m := lineNumberPointRE.FindStringSubmatch(p); m != nil {
		line, err := strconv.Atoi(m[1])
		if err != nil {
			return nil, fmt.Errorf("invalid line number: %v", err)
		}
		np := &cpb.Point{LineNumber: int32(line)}
		if m[3] != "" {
			col, err := strconv.Atoi(m[3])
			if err != nil {
				return nil, fmt.Errorf("invalid column offset: %v", err)
			}
			np.ColumnOffset = int32(col)
		}
		return np, nil
	}
	return nil, fmt.Errorf("unknown format %q", p)
}

func parseSpan(span string) (*cpb.Span, error) {
	parts := strings.Split(span, "-")
	if len(parts) != 2 {
		return nil, errors.New("unknown format")
	}
	start, err := parsePoint(parts[0])
	if err != nil {
		return nil, fmt.Errorf("invalid start: %v", err)
	}
	end, err := parsePoint(parts[1])
	if err != nil {
		return nil, fmt.Errorf("invalid end: %v", err)
	}
	return &cpb.Span{
		Start: start,
		End:   end,
	}, nil
}

type sourceCommand struct {
	decorSpan string
}

func (sourceCommand) Name() string     { return "source" }
func (sourceCommand) Synopsis() string { return "retrieve a file's source text" }
func (sourceCommand) Usage() string    { return "" }
func (c *sourceCommand) SetFlags(flag *flag.FlagSet) {
	flag.StringVar(&c.decorSpan, "span", "", spanHelp)
	flag.StringVar(&DefaultFileCorpus, "corpus", DefaultFileCorpus, "File corpus to use if given a raw path")
}
func (c sourceCommand) Run(ctx context.Context, flag *flag.FlagSet, api API) error {
	ticket, err := fileTicketArg(flag)
	if err != nil {
		return err
	}
	req := &xpb.DecorationsRequest{
		Location:   &xpb.Location{Ticket: ticket},
		SourceText: true,
	}
	if c.decorSpan != "" {
		span, err := parseSpan(c.decorSpan)
		if err != nil {
			return fmt.Errorf("invalid --span %q: %v", c.decorSpan, err)
		}

		req.Location.Kind = xpb.Location_SPAN
		req.Location.Span = span
	}

	logRequest(req)
	reply, err := api.XRefService.Decorations(ctx, req)
	if err != nil {
		return err
	}
	return displaySource(reply)
}

func displaySource(decor *xpb.DecorationsReply) error {
	if *displayJSON {
		return jsonMarshaler.Marshal(out, decor)
	}

	_, err := out.Write(decor.SourceText)
	return err
}
