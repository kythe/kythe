/*
 * Copyright 2015 Google Inc. All rights reserved.
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

// Package process contains an implementation of a callback-based asynchronous
// process.
package process

import (
	"os"
	"os/exec"
)

// Async controls an asynchronous process with callbacks to handle its exit.
// Each callback can be nil to ignore that particular process state.
type Async struct {
	// Command to run asynchronously
	Command *exec.Cmd

	// OnStart is called once the process has been started.
	OnStart func(p *os.Process)
	// OnExit will be called, if non-nil, once the process exits (with or without
	// an error).  OnExit is called after any call to either OnSuccess or OnError.
	OnExit func(state *os.ProcessState, err error)

	// OnSuccess will be called if it is non-nil and the process exits
	// successfully.
	OnSuccess func(state *os.ProcessState)
	// OnError will be called if it is non-nil and the process exits with an
	// error.
	OnError func(state *os.ProcessState, err error)
}

// Start runs the specified command in the background and calls the provided
// handlers once the command exits.
func (a *Async) Start() {
	err := a.Command.Start()
	if a.OnStart != nil {
		a.OnStart(a.Command.Process)
	}
	go func() {
		if err != nil {
			a.runHandlers(err)
			return
		}
		a.runHandlers(a.Command.Wait())
	}()
}

func (a *Async) runHandlers(err error) {
	state := a.Command.ProcessState
	if err == nil {
		if a.OnSuccess != nil {
			a.OnSuccess(state)
		}
	} else {
		if a.OnError != nil {
			a.OnError(state, err)
		}
	}
	if a.OnExit != nil {
		a.OnExit(state, err)
	}
}
