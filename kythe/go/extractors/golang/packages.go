/*
 * Copyright 2018 The Kythe Authors. All rights reserved.
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

package golang

import (
	"bytes"
	"encoding/json"
	"fmt"
	"go/build"
	"os"
	"os/exec"
	"path/filepath"
	"strings"
)

// Fields must match go list;
// see $GOROOT/src/cmd/go/internal/load/pkg.go.
type jsonPackage struct {
	Dir        string
	ImportPath string
	Name       string
	Doc        string
	Root       string
	Export     string
	Goroot     bool

	GoFiles      []string
	CFiles       []string
	CgoFiles     []string
	CXXFiles     []string
	MFiles       []string
	HFiles       []string
	FFiles       []string
	SFiles       []string
	SwigFiles    []string
	SwigCXXFiles []string
	SysoFiles    []string

	CgoCFLAGS    []string
	CgoCPPFLAGS  []string
	CgoCXXFLAGS  []string
	CgoFFLAGS    []string
	CgoLDFLAGS   []string
	CgoPkgConfig []string

	Imports []string

	TestGoFiles  []string
	TestImports  []string
	XTestGoFiles []string
	XTestImports []string

	ForTest string // q in a "p [q.test]" package, else ""
	DepOnly bool

	Error *jsonPackageError
}

func (pkg *jsonPackage) buildPackage() *build.Package {
	bp := &build.Package{
		Dir:        pkg.Dir,
		ImportPath: pkg.ImportPath,
		Name:       pkg.Name,
		Doc:        pkg.Doc,
		Root:       pkg.Root,
		PkgObj:     pkg.Export,
		Goroot:     pkg.Goroot,

		GoFiles:      pkg.GoFiles,
		CgoFiles:     pkg.CgoFiles,
		CFiles:       pkg.CFiles,
		CXXFiles:     pkg.CXXFiles,
		MFiles:       pkg.MFiles,
		HFiles:       pkg.HFiles,
		FFiles:       pkg.FFiles,
		SFiles:       pkg.SFiles,
		SwigFiles:    pkg.SwigFiles,
		SwigCXXFiles: pkg.SwigCXXFiles,
		SysoFiles:    pkg.SysoFiles,

		CgoCFLAGS:    pkg.CgoCFLAGS,
		CgoCPPFLAGS:  pkg.CgoCPPFLAGS,
		CgoCXXFLAGS:  pkg.CgoCXXFLAGS,
		CgoFFLAGS:    pkg.CgoFFLAGS,
		CgoLDFLAGS:   pkg.CgoLDFLAGS,
		CgoPkgConfig: pkg.CgoPkgConfig,

		Imports: pkg.Imports,

		TestGoFiles:  pkg.TestGoFiles,
		TestImports:  pkg.TestImports,
		XTestGoFiles: pkg.XTestGoFiles,
		XTestImports: pkg.XTestImports,
	}
	if bp.Root != "" {
		bp.SrcRoot = filepath.Join(bp.Root, "src")
		bp.PkgRoot = filepath.Join(bp.Root, "pkg")
		bp.BinDir = filepath.Join(bp.Root, "bin")
	}
	return bp
}

type jsonPackageError struct {
	ImportStack []string
	Pos         string
	Err         string
}

func (e jsonPackageError) Error() string { return fmt.Sprintf("%s: %s", e.Pos, e.Err) }

func buildContextEnv(bc build.Context) ([]string, error) {
	cgo := "0"
	if bc.CgoEnabled {
		cgo = "1"
	}
	vars := []string{
		"GO111MODULE=off",
		"CGO_ENABLED=" + cgo,
		"GOARCH=" + bc.GOARCH,
		"GOOS=" + bc.GOOS,
	}
	envPaths := map[string]string{
		"GOROOT": bc.GOROOT,
		"GOPATH": bc.GOPATH,
	}
	for name, path := range envPaths {
		abs, err := filepath.Abs(path)
		if err != nil {
			return nil, fmt.Errorf("error finding absolute path for %q: %v", path, err)
		}
		vars = append(vars, fmt.Sprintf("%s=%s", name, abs))
	}
	return vars, nil
}

func (e *Extractor) listPackages(query ...string) ([]*jsonPackage, error) {
	// TODO(schroederc): support GOPACKAGESDRIVER
	args := append([]string{"list",
		"-compiler=" + e.BuildContext.Compiler,
		"-tags=" + strings.Join(e.BuildContext.BuildTags, " "),
		"-installsuffix=" + e.BuildContext.InstallSuffix,
		"-test",
		"-deps",
		"-e",
		"-export",
		"-compiled",
		"-json",
		"--"}, query...)
	goTool := "go"
	if e.BuildContext.GOROOT != "" {
		goTool = filepath.Join(e.BuildContext.GOROOT, "bin/go")
	}
	cmd := exec.Command(goTool, args...)
	env, err := buildContextEnv(e.BuildContext)
	if err != nil {
		return nil, err
	}
	cmd.Env = append(os.Environ(), env...)
	var out bytes.Buffer
	cmd.Stdout = &out
	cmd.Stderr = os.Stderr
	listErr := cmd.Run()

	var pkgs []*jsonPackage
	for de := json.NewDecoder(&out); de.More(); {
		var pkg jsonPackage
		if err := de.Decode(&pkg); err != nil {
			return nil, err
		}
		pkgs = append(pkgs, &pkg)
	}
	return pkgs, listErr
}
