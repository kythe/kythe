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
	"regexp"
	"strconv"
	"strings"

	cpb "kythe.io/kythe/proto/common_go_proto"
	xpb "kythe.io/kythe/proto/xref_go_proto"
)

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

func parseLine(p string) (*cpb.Span, error) {
	m := lineNumberPointRE.FindStringSubmatch(p)
	if m == nil {
		return nil, errors.New("unknown format")
	}
	line, err := strconv.Atoi(m[1])
	if err != nil {
		return nil, fmt.Errorf("invalid line number: %v", err)
	}
	start := &cpb.Point{LineNumber: int32(line)}
	if m[3] != "" {
		col, err := strconv.Atoi(m[3])
		if err != nil {
			return nil, fmt.Errorf("invalid column offset: %v", err)
		}
		start.ColumnOffset = int32(col)
	}
	return &cpb.Span{
		Start: start,
		End:   &cpb.Point{LineNumber: start.LineNumber + 1},
	}, nil
}

func parseSpan(span string) (*cpb.Span, error) {
	parts := strings.Split(span, "-")
	if len(parts) == 1 {
		return parseLine(parts[0])
	} else if len(parts) != 2 {
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
	baseDecorCommand
}

func (sourceCommand) Name() string     { return "source" }
func (sourceCommand) Synopsis() string { return "retrieve a file's source text" }
func (sourceCommand) Usage() string    { return "" }
func (c *sourceCommand) SetFlags(flag *flag.FlagSet) {
	c.baseDecorCommand.SetFlags(flag)
}
func (c sourceCommand) Run(ctx context.Context, flag *flag.FlagSet, api API) error {
	req, err := c.baseRequest(flag)
	if err != nil {
		return err
	}
	req.SourceText = true

	LogRequest(req)
	reply, err := api.XRefService.Decorations(ctx, req)
	if err != nil {
		return err
	}
	return displaySource(reply)
}

func displaySource(decor *xpb.DecorationsReply) error {
	if DisplayJSON {
		return PrintJSONMessage(decor)
	}

	_, err := out.Write(decor.SourceText)
	return err
}
