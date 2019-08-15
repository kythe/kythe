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
	"path/filepath"
	"time"

	"kythe.io/kythe/go/languageserver"
	"kythe.io/kythe/go/services/xrefs"

	"github.com/sourcegraph/jsonrpc2"
)

var (
	pageSize = flag.Int("page_size", 0, "Set the default xrefs page size")

	serverAddr = flag.String("server", "localhost:8080",
		"The address of the Kythe service to use (:8080 allows access from other machines)")
)

func main() {
	flag.Parse()
	if *serverAddr == "" {
		log.Fatal("You must provide a --server address")
	}

	// Set up the log file
	const openMode = os.O_CREATE | os.O_APPEND | os.O_WRONLY
	logfile := fmt.Sprintf("kythe-ls-%s.log", time.Now().Format("20160102-150405"))
	file, err := os.OpenFile(filepath.Join(os.TempDir(), logfile), openMode, os.ModePerm)
	if err != nil {
		log.Fatalf("Unable to create log file: %v", err)
	}
	log.SetOutput(file)

	// Check to see that xref service is reachable. We won't hold open a
	// connection to the server here, as the client manages the connection.
	conn, err := net.DialTimeout("tcp", *serverAddr, 5*time.Second)
	if err != nil {
		log.Fatalf("Dialing Kythe service: %v", err)
	}
	conn.Close()

	client := xrefs.WebClient("http://" + *serverAddr)
	server := languageserver.NewServer(client, &languageserver.Options{
		PageSize: *pageSize,
	})

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
