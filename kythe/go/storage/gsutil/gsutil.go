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

// Package gsutil is collection of helper functions for storage tools.
package gsutil

import (
	"errors"
	"flag"
	"fmt"
	"log"
	"os"
	"os/signal"
	"strings"
	"syscall"

	"kythe/go/services/graphstore"
	"kythe/go/services/graphstore/proxy"
	"kythe/go/storage/inmemory"
	"kythe/go/storage/leveldb"
	"kythe/go/storage/sql"
)

type gsFlag struct {
	gs *graphstore.Service
}

// String implements part of the flag.Value interface.
func (f *gsFlag) String() string { return fmt.Sprintf("%T", *f.gs) }

// Set implements part of the flag.Value interface.
func (f *gsFlag) Set(str string) (err error) {
	*f.gs, err = ParseGraphStore(str)
	return
}

// Flag defines a GraphStore flag with the specified name and usage string.
func Flag(gs *graphstore.Service, name, usage string) {
	if gs == nil {
		log.Fatalf("GraphStoreFlag given nil GraphStore pointer")
	}
	f := gsFlag{gs: gs}
	flag.Var(&f, name, usage)
}

// ParseGraphStore returns a GraphStore for the given specification.
func ParseGraphStore(str string) (graphstore.Service, error) {
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
		case leveldb.ValidDB(spec):
			kind = "leveldb"
		default:
			return nil, fmt.Errorf("unknown GraphStore: %q", str)
		}
	}

	switch kind {
	case "in-memory":
		return inmemory.Create(), nil
	case "proxy":
		var stores []graphstore.Service
		for _, s := range strings.Split(spec, ",") {
			gs, err := ParseGraphStore(s)
			if err != nil {
				return nil, fmt.Errorf("proxy GraphStore error for %q: %v", s, err)
			}
			stores = append(stores, gs)
		}
		if len(stores) == 0 {
			return nil, errors.New("no proxy GraphStores specified")
		}
		return proxy.New(stores...), nil
	case "leveldb":
		gs, err := leveldb.OpenGraphStore(spec, nil)
		if err != nil {
			return nil, fmt.Errorf("error opening LevelDB GraphStore: %v", err)
		}
		return gs, nil
	default:
		gs, err := sql.OpenGraphStore(kind, spec)
		if err != nil {
			return nil, fmt.Errorf("error opening SQL GraphStore: %v", err)
		}
		return gs, nil
	}
}

// EnsureGracefulExit will try to close each gs when notified of an Interrupt,
// SIGTERM, or Kill signal and immediately exit the program unsuccessfully. Any
// errors will be logged. This function should only be called once and closing
// the GraphStores manually is still needed when the program does not receive a
// signal to quit.
func EnsureGracefulExit(gs ...graphstore.Service) {
	c := make(chan os.Signal, 1)
	signal.Notify(c, os.Interrupt)
	signal.Notify(c, syscall.SIGTERM)
	signal.Notify(c, os.Kill)
	go func() {
		sig := <-c
		log.Printf("graphstore: signal %v", sig)
		for _, g := range gs {
			LogClose(g)
		}
		os.Exit(1)
	}()
}

// LogClose closes gs and logs any resulting error.
func LogClose(gs graphstore.Service) {
	if err := gs.Close(); err != nil {
		log.Printf("GraphStore failed to close: %v", err)
	}
}

// UsageError prints msg to stderr, calls flag.Usage, and exits the program
// unsuccessfully.
func UsageError(msg string) {
	fmt.Fprintln(os.Stderr, msg)
	flag.Usage()
	os.Exit(1)
}

// UsageErrorf prints str formatted with the given vals to stderr, calls
// flag.Usage, and exits the program unsuccessfully.
func UsageErrorf(str string, vals ...interface{}) {
	UsageError(fmt.Sprintf(str, vals...))
}
