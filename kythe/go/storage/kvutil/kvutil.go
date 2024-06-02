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

// Package kvutil is collection of helper functions for storage tools.
package kvutil // import "kythe.io/kythe/go/storage/kvutil"

import (
	"context"
	"flag"
	"fmt"
	"os"
	"os/signal"
	"strings"
	"syscall"

	"kythe.io/kythe/go/storage/keyvalue"
	"kythe.io/kythe/go/util/log"
)

// Handler returns a keyvalue.DB based on the given specification.
// See also: Register(string, Handler).
type Handler func(spec string) (keyvalue.DB, error)

var (
	handlers           = map[string]Handler{}
	defaultHandlerKind string
)

// Register exposes the given Handler to ParseDB.  Each string starting
// with kind+":" will be passed to the given Handler.  A kind can only be
// registered once.
func Register(kind string, h Handler) {
	if _, exists := handlers[kind]; exists {
		log.Fatalf("kvutil Handler for kind %q already exists", kind)
	}
	handlers[kind] = h
}

// RegisterDefault gives ParseDB a fallback kind if not given any
// "____:" prefix.  A default can only be set once.
func RegisterDefault(kind string) {
	if defaultHandlerKind != "" {
		log.Fatalf("default kvutil Handler kind already registered as %q", defaultHandlerKind)
	}
	defaultHandlerKind = kind
}

type gsFlag struct {
	gs *keyvalue.DB
}

// String implements part of the flag.Value interface.
func (f *gsFlag) String() string {
	if f.gs == nil {
		return "<graphstore>"
	}
	return fmt.Sprintf("%T", *f.gs)
}

// Get implements part of the flag.Getter interface.
func (f *gsFlag) Get() any {
	return *f.gs
}

// Set implements part of the flag.Value interface.
func (f *gsFlag) Set(str string) (err error) {
	*f.gs, err = ParseDB(str)
	return
}

// Flag defines a DB flag with the specified name and usage string.
func Flag(gs *keyvalue.DB, name, usage string) {
	if gs == nil {
		log.Fatal("DBFlag given nil DB pointer")
	}
	f := gsFlag{gs: gs}
	flag.Var(&f, name, usage)
}

// ParseDB returns a DB for the given specification.
func ParseDB(str string) (keyvalue.DB, error) {
	str = strings.TrimSpace(str)
	split := strings.SplitN(str, ":", 2)
	var kind, spec string
	if len(split) == 2 {
		spec = split[1]
		kind = split[0]
	} else {
		spec = str
		switch {
		case spec == "" || spec == "in-memory":
			kind = "in-memory"
		case defaultHandlerKind != "":
			kind = defaultHandlerKind
		default:
			return nil, fmt.Errorf("unknown DB: %q", str)
		}
	}

	h, ok := handlers[kind]
	if !ok {
		return nil, fmt.Errorf("no kvutil Handler registered for kind %q", kind)
	}
	return h(spec)
}

// EnsureGracefulExit will try to close each gs when notified of an Interrupt,
// SIGTERM, or Kill signal and immediately exit the program unsuccessfully. Any
// errors will be logged. This function should only be called once and closing
// the DBs manually is still needed when the program does not receive a
// signal to quit.
func EnsureGracefulExit(gs ...keyvalue.DB) {
	c := make(chan os.Signal, 1)
	signal.Notify(c, os.Interrupt)
	signal.Notify(c, syscall.SIGTERM)
	go func() {
		sig := <-c
		log.Infof("db: signal %v", sig)
		for _, g := range gs {
			LogClose(context.Background(), g)
		}
		os.Exit(1)
	}()
}

// LogClose closes gs and logs any resulting error.
func LogClose(ctx context.Context, gs keyvalue.DB) {
	if err := gs.Close(ctx); err != nil {
		log.InfoContextf(ctx, "DB failed to close: %v", err)
	}
}
