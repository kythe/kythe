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

// Package process contains an implementation of a callback-based asynchronous
// process.
package process // import "kythe.io/kythe/go/util/process"

import (
	"os"
	"os/exec"
)

// Callbacks is a set of callbacks to be called throughout a process' lifetime.
// Each callback can be nil to ignore that particular process state.
type Callbacks struct {
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

// StartAsync runs the specified command in the background and calls the
// provided handlers during the proccess' lifetime.  If there is an error
// starting the process, it is returned and none of the callbacks are
// called.
func StartAsync(cmd *exec.Cmd, c *Callbacks) error {
	if err := cmd.Start(); err != nil {
		return err
	}

	if c != nil {
		if c.OnStart != nil {
			c.OnStart(cmd.Process)
		}
		go func() { c.run(cmd, cmd.Wait()) }()
	}

	return nil
}

func (c *Callbacks) run(cmd *exec.Cmd, err error) {
	state := cmd.ProcessState
	if err == nil {
		if c.OnSuccess != nil {
			c.OnSuccess(state)
		}
	} else {
		if c.OnError != nil {
			c.OnError(state, err)
		}
	}
	if c.OnExit != nil {
		c.OnExit(state, err)
	}
}
