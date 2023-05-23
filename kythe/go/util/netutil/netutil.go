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

// Package netutil provides network utility functions.
package netutil // import "kythe.io/kythe/go/util/netutil"

import (
	"fmt"
	"net"
	"strconv"

	"kythe.io/kythe/go/util/log"
)

// PickUnusedPort returns a port that is likely unused by temporarily binding a
// port by listening on localhost:0 and determining the port given to the
// process.
func PickUnusedPort() (int, error) {
	l, err := net.Listen("tcp", "localhost:0")
	if err != nil {
		return 0, err
	}
	if err := l.Close(); err != nil {
		log.Errorf("failed to close temporary Listener: %v", err)
	}
	_, port, err := net.SplitHostPort(l.Addr().String())
	if err != nil {
		return 0, fmt.Errorf("failed to parse address %q: %v", l.Addr().String(), err)
	}
	return strconv.Atoi(port)
}
