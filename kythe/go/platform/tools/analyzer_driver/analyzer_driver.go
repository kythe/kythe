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

// Binary analyzer_driver drives a CompilationAnalyzer server as a subprocess.
// Compilations given on the command-line (.kindex files) are sent to the
// analyzer and all results are written as a delimited stream to stdout.
//
// See --help for more information.
package main

import (
	"context"
	"errors"
	"flag"
	"fmt"
	"log"
	"net"
	"os"
	"os/exec"
	"strconv"
	"strings"

	"kythe.io/kythe/go/platform/analysis"
	"kythe.io/kythe/go/platform/analysis/driver"
	"kythe.io/kythe/go/platform/analysis/local"
	"kythe.io/kythe/go/platform/analysis/remote"
	"kythe.io/kythe/go/platform/delimited"
	"kythe.io/kythe/go/util/flagutil"
	"kythe.io/kythe/go/util/netutil"
	"kythe.io/kythe/go/util/process"

	"google.golang.org/grpc"

	apb "kythe.io/kythe/proto/analysis_proto"
	aspb "kythe.io/kythe/proto/analysis_service_proto"
)

func init() {
	flag.Usage = flagutil.SimpleUsage(`Local CompilationAnalyzer server driver

Drives a CompilationAnalyzer server as a subprocess, sending it
AnalysisRequests, and writing the AnalysisOutput values as a delimited stream.

The command for the analyzer is given as non-flag arguments with the string
@port@ replaced with --analyzer_port.`,
		`[--analyzer_port int]
<analyzer-command> [analyzer-args...] -- <kindex-file...>`)
}

var (
	analyzerPort = flag.Int("analyzer_port", 0, "Listening port of analyzer server (0 indicates to pick an unused port)")
	fdsPort      = flag.Int("fds_port", 0, "Listening port for local FileDataService server (0 indicates to pick an unused port)")
)

func main() {
	flag.Parse()

	// done is sent a value when the analyzer should exit
	done := make(chan struct{}, 1)
	defer func() { done <- struct{}{} }()

	analyzerBin, analyzerArgs, compilations := parseAnalyzerCommand()
	if len(compilations) == 0 {
		flagutil.UsageError("Missing kindex-file paths")
	}

	cmd := exec.Command(analyzerBin, analyzerArgs...)
	cmd.Stdout = os.Stdout
	cmd.Stderr = os.Stderr
	var proc *os.Process
	if err := process.StartAsync(cmd, &process.Callbacks{
		OnStart: func(p *os.Process) {
			log.Printf("Starting analyzer subprocess: %s", strings.Join(cmd.Args, " "))
			proc = p
		},
		OnExit: func(state *os.ProcessState, err error) {
			select {
			case <-done:
			default:
				log.Fatalf("Analyzer subprocess exited unexpectedly (state:%v; error:%v)", state, err)
			}
		},
	}); err != nil {
		log.Fatalf("Error starting analyzer: %v", err)
	}

	addr := fmt.Sprintf("localhost:%d", *analyzerPort)
	conn, err := grpc.Dial(addr, grpc.WithInsecure())
	if err != nil {
		log.Fatalf("Error dialing analyzer %q: %v", addr, err)
	}
	defer conn.Close()

	queue := local.NewKIndexQueue(compilations)
	fdsAddr := launchFileDataService(queue)

	wr := delimited.NewWriter(os.Stdout)

	driver := &driver.Driver{
		Analyzer:        &remote.Analyzer{aspb.NewCompilationAnalyzerClient(conn)},
		FileDataService: fdsAddr,
		WriteOutput: func(_ context.Context, out *apb.AnalysisOutput) error {
			return wr.Put(out.Value)
		},
	}

	if err := driver.Run(context.Background(), queue); err != nil {
		log.Fatal(err)
	}

	if err := proc.Signal(os.Interrupt); err != nil {
		log.Fatalf("Failed to send interrupt to analyzer: %v", err)
	}
}

// launchFileDataService starts up a local FileDataService and returns its
// address.  In case of error the program is terminated.
func launchFileDataService(f analysis.Fetcher) string {
	fds := &fileDataService{fetcher: f}
	srv := grpc.NewServer()
	l, err := net.Listen("tcp", "localhost:"+strconv.Itoa(*fdsPort))
	if err != nil {
		log.Fatalf("Error binding listening port for FileDataService: %v", err)
	}
	aspb.RegisterFileDataServiceServer(srv, fds)
	go func() { log.Fatal(srv.Serve(l)) }()
	return l.Addr().String()
}

// parseAnalyzerCommand extracts the analyzer subcommand line from the current
// process's argument list, and replaces any occurrences of "@port@" in its
// command-line with the designated analyzer port. If no port is given, one is
// chosen and stored into *analyzerPort.
//
// The command and its arguments are returned, along with any trailing
// unclaimed arguments to use as compilation unit sources.
func parseAnalyzerCommand() (command string, args, compilations []string) {
	if *analyzerPort <= 0 {
		picked, err := netutil.PickUnusedPort()
		if err != nil {
			log.Fatalf("Failed to pick analyzer port: %v", err)
		}
		*analyzerPort = picked
	}

	// Find the breakpoint between the subcommand arguments and the trailing
	// arguments, if any.
	port := strings.NewReplacer("@port@", strconv.Itoa(*analyzerPort))
	for i, arg := range flag.Args() {
		if i == 0 {
			command = arg
		} else if arg == "--" {
			compilations = flag.Args()[i+1:]
			break
		} else {
			args = append(args, port.Replace(arg))
		}
	}
	return
}

// fileDataService implements the apb.FileDataServiceServer interface backed by
// a single analysis.Fetcher.
type fileDataService struct {
	fetcher analysis.Fetcher
}

// Get implements the aspb.FileDataServiceServer interface.
func (s *fileDataService) Get(req *apb.FilesRequest, srv aspb.FileDataService_GetServer) error {
	for _, info := range req.Files {
		if info.Path == "" && info.Digest == "" {
			return errors.New("file request missing both path and digest")
		}
	}
	for _, info := range req.Files {
		data, err := s.fetcher.Fetch(info.Path, info.Digest)
		if err != nil {
			// Report the file as missing.
			if serr := srv.Send(&apb.FileData{
				Info:    info,
				Missing: true,
			}); serr != nil {
				return serr
			}
			continue
		}
		if err := srv.Send(&apb.FileData{
			Content: data,
			Info:    info,
		}); err != nil {
			return err
		}
	}
	return nil
}
