/*
 * Copyright 2016 The Kythe Authors. All rights reserved.
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

// Package nodes defines constants for Kythe nodes.
package nodes // import "kythe.io/kythe/go/util/schema/nodes"

// Node kind labels
const (
	Abs        = "abs"
	Anchor     = "anchor"
	Constant   = "constant"
	Diagnostic = "diagnostic"
	Doc        = "doc"
	File       = "file"
	Function   = "function"
	Interface  = "interface"
	Name       = "name"
	Package    = "package"
	Record     = "record"
	Symbol     = "symbol"
	TAlias     = "talias"
	TApp       = "tapp"
	TBuiltin   = "tbuiltin"
	TNominal   = "tnominal"
	Variable   = "variable"
)

// Node subkinds
const (
	Class          = "class"
	Enum           = "enum"
	EnumClass      = "enumClass"
	Field          = "field"
	Implicit       = "implicit"
	Local          = "local"
	LocalParameter = "local/parameter"
	Struct         = "struct"
	Type           = "type"
	Union          = "union"
)
