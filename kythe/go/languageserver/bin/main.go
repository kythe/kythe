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

// Binary kythe_languageserver provides a Language Server Protocol v3
// implementation for Kythe indexes that communicates via JSONRPC2.0 over stdio
package main

import (
	"context"
	"flag"
	"fmt"
	"log"
	"net"
	"os"
	"time"

	"kythe.io/kythe/go/languageserver/languageserver"
	"kythe.io/kythe/go/services/xrefs"

	"github.com/sourcegraph/jsonrpc2"
)

var (
	host = flag.String("host", "localhost", "Host for the Kythe xref service")
	port = flag.Int("port", 8080, "Host for the Kythe xref service")
)

func main() {
	flag.Parse()

	host := fmt.Sprintf("%s:%d", *host, *port)

	// Check to see that xref service is reachable
	conn, err := net.DialTimeout("tcp", host, 5*time.Second)
	if err != nil {
		log.Fatalf("XRef service unreachable")
	}
	conn.Close()

	client := xrefs.WebClient("http://" + host)
	server := languageserver.NewServer(client, languageserver.SettingsPathConfigProvider)

	<-jsonrpc2.NewConn(
		context.Background(),
		jsonrpc2.NewBufferedStream(stdio{}, jsonrpc2.VSCodeObjectCodec{}),
		languageserver.ServerHandler(&server),
	).DisconnectNotify()
}

type stdio struct{}

func (stdio) Read(data []byte) (int, error)  { return os.Stdin.Read(data) }
func (stdio) Write(data []byte) (int, error) { return os.Stdout.Write(data) }
func (stdio) Close() error                   { return os.Stdout.Close() }
