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

package process

import (
	"os"
	"os/exec"
	"strings"
	"testing"
	"time"
)

func init() {
	// Ensure path contains at least the usual locations for true and false.
	os.Setenv("PATH", strings.Join([]string{
		os.Getenv("PATH"),
		"/bin",
		"/usr/bin",
	}, string(os.PathListSeparator)))
}

func find(t *testing.T, file string) string {
	p, err := exec.LookPath(file)
	if err != nil {
		t.Fatal(err)
	}
	return p
}

func TestAsyncSuccess(t *testing.T) {
	done := make(chan struct{}, 1)
	a := &Async{
		Command: exec.Command(find(t, "true")),
		OnStart: func(p *os.Process) {
			if p == nil {
				t.Error("Process was nil")
			}
			done <- struct{}{}
		},
		OnExit: func(state *os.ProcessState, err error) {
			if err != nil {
				t.Errorf("Process exited with unknown error: %v (%v)", err, state)
			}
			done <- struct{}{}
		},
		OnSuccess: func(state *os.ProcessState) {
			if !state.Exited() {
				t.Errorf("Process did not exit: %v", state)
			}
			if !state.Success() {
				t.Errorf("Process was not successful: %v", state)
			}
			done <- struct{}{}
		},
		OnError: func(state *os.ProcessState, err error) {
			t.Errorf("Process exited with unknown error: %v (%v)", err, state)
		},
	}
	a.Start()

	timeout := time.After(5 * time.Second)
	for i := 0; i < 3; i++ {
		select {
		case <-done:
		case <-timeout:
			t.Fatal("Process did not finish before timeout")
		}
	}
}

func TestAsyncError(t *testing.T) {
	done := make(chan struct{}, 1)
	a := &Async{
		Command: exec.Command(find(t, "false")),
		OnStart: func(p *os.Process) {
			if p == nil {
				t.Error("Process was nil")
			}
			done <- struct{}{}
		},
		OnExit: func(state *os.ProcessState, err error) {
			if err == nil {
				t.Errorf("Process exited with no error: %v", state)
			}
			done <- struct{}{}
		},
		OnSuccess: func(state *os.ProcessState) {
			t.Errorf("Process was unexpectedly successful: %v", state)
		},
		OnError: func(state *os.ProcessState, err error) {
			if state.Success() {
				t.Errorf("Process was unexpectedly successful: %v", state)
			}
			if !state.Exited() {
				t.Errorf("Process did not exit: %v", state)
			}
			done <- struct{}{}
		},
	}
	a.Start()

	timeout := time.After(5 * time.Second)
	for i := 0; i < 3; i++ {
		select {
		case <-done:
		case <-timeout:
			t.Fatal("Process did not finish before timeout")
		}
	}
}

func TestAsyncNilHandlers(t *testing.T) {
	bt := &Async{Command: exec.Command(find(t, "true"))}
	bt.Start()
	bt.Command.Wait()

	bf := &Async{Command: exec.Command(find(t, "false"))}
	bf.Start()
	bf.Command.Wait()
}
