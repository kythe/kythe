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

// Package oauth2 implements a utility Config that can be made a flag and turned
// into a proper Context or http Client for use with Google apis.
package oauth2

import (
	"errors"
	"flag"
	"fmt"
	"io/ioutil"
	"net/http"
	"strings"

	"kythe.io/kythe/go/platform/vfs"

	"golang.org/x/net/context"
	"golang.org/x/oauth2"
	"golang.org/x/oauth2/google"
	"google.golang.org/cloud"
)

// Config represents an oauth2 configuration, usually in the context of Google
// services.
type Config struct {
	ProjectID  string
	GCEAccount string

	ConfigFile string
	Scopes     []string
}

type stringListVar struct {
	s *[]string
}

// String implements part of the flag.Value interface.
func (v stringListVar) String() string {
	return strings.Join(*v.s, ",")
}

// Set implements part of the flag.Value interface.
func (v stringListVar) Set(str string) error {
	*v.s = strings.Split(str, ",")
	return nil
}

// NewConfigFlags returns a Config with fields exposed by flags.
func NewConfigFlags(flag *flag.FlagSet) *Config {
	c := &Config{}
	flag.StringVar(&c.GCEAccount, "gce_account", "", "GCE account to use for oauth2")
	flag.StringVar(&c.ProjectID, "project_id", "", "Google cloud project ID")
	flag.StringVar(&c.ConfigFile, "oauth2_config", "", "Path to oauth2 JSON configuration")
	flag.Var(stringListVar{&c.Scopes}, "oauth2_scopes", "Scopes for oauth2")
	return c
}

// Context returns an cloud.Context based on the receiver Config.
func (c *Config) Context(ctx context.Context) (context.Context, error) {
	if c.ProjectID == "" {
		return nil, errors.New("oauth2 configuration missing project ID")
	}
	cl, err := c.Client(ctx)
	if err != nil {
		return nil, err
	}
	return cloud.WithContext(ctx, c.ProjectID, cl), nil
}

// Client returns an oauth2-based http.Client based on the receiver Config.
func (c *Config) Client(ctx context.Context) (*http.Client, error) {
	switch {
	case c.GCEAccount != "":
		return oauth2.NewClient(ctx, google.ComputeTokenSource(c.GCEAccount)), nil
	case c.ConfigFile != "":
		f, err := vfs.Open(ctx, c.ConfigFile)
		if err != nil {
			return nil, fmt.Errorf("error opening ConfigFile: %v", err)
		}
		data, err := ioutil.ReadAll(f)
		f.Close()
		if err != nil {
			return nil, fmt.Errorf("error reading ConfigFile: %v", err)
		}
		config, err := google.JWTConfigFromJSON(data, c.Scopes...)
		if err != nil {
			return nil, fmt.Errorf("JWT config error: %v", err)
		}
		return config.Client(ctx), nil
	default:
		return nil, errors.New("oauth2 misconfigured")
	}
}
