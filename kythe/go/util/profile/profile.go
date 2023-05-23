/*
 * Copyright 2015 The Kythe Authors. All rights reserved.
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

// Package profile provides a simple method for exposing profile information
// through a --cpu_profile flag.  This package also implicitly adds the
// /debug/pprof/... HTTP handlers.
package profile // import "kythe.io/kythe/go/util/profile"

import (
	"context"
	"errors"
	"flag"
	"fmt"
	"io"
	"os"
	"path/filepath"
	"runtime/pprof"
	"strings"
	"sync"

	"kythe.io/kythe/go/platform/vfs"
	"kythe.io/kythe/go/util/log"

	_ "net/http/pprof" // for /debug/pprof/... handlers
)

var (
	profCPU = flag.String("cpu_profile", "", "Write CPU profile to the specified file (if nonempty)")

	file io.Closer
	mu   sync.Mutex
)

// Start begins profiling the program, writing data to the file given with the
// --cpu_profile flag.  If --cpu_profile was not given, nothing happens.  Start
// must not be called again until Stop is called.  The profile data in the
// --cpu_profile flag is overwritten on each call to Start.
func Start(ctx context.Context) error {
	mu.Lock()
	defer mu.Unlock()

	if *profCPU != "" {
		if file != nil {
			return errors.New("profiling already started")
		}

		f, err := vfs.Create(ctx, *profCPU)
		if err != nil {
			return fmt.Errorf("error creating profile file %q: %v", *profCPU, err)
		}
		file = f
		pprof.StartCPUProfile(f)
	}
	return nil
}

// Stop stops profiling the program.  If --cpu_profile was not given, nothing
// happens.  Start must have be called before a call to Stop.
func Stop() error {
	mu.Lock()
	defer mu.Unlock()

	if *profCPU != "" {
		if file == nil {
			return errors.New("profiling hasn't started")
		}

		pprof.StopCPUProfile()
		err := file.Close()
		file = nil
		if err != nil {
			return err
		}

		binPath := os.Args[0]
		profPath := *profCPU

		// Try to shorten file paths, if possible
		cwd, err := os.Getwd()
		if err == nil {
			relBin, err := filepath.Rel(cwd, binPath)
			if err == nil && !strings.HasPrefix(relBin, "../") {
				binPath = relBin
			}
			relProf, err := filepath.Rel(cwd, profPath)
			if err == nil && !strings.HasPrefix(relProf, "../") {
				profPath = relProf
			}
		}

		log.Infof("Profile data written: go tool pprof %s %s", binPath, profPath)
	}
	return nil
}
