/*
 * Copyright 2023 The Kythe Authors. All rights reserved.
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

// Package log provides semantic log functions.
package log

import (
	"context"
	"log"
	"reflect"
)

// Infof logs to the informational log.
func Infof(msg string, args ...any) { log.Printf(msg, args...) }

// Info logs to the informational log.
func Info(args ...any) { log.Println(args...) }

// Warningf logs to the warning log.
func Warningf(msg string, args ...any) { log.Printf("WARNING: "+msg, args...) }

// Warning logs to the warning log.
func Warning(args ...any) { log.Print(append([]any{"WARNING:"}, args...)) }

// Errorf logs to the error log.
func Errorf(msg string, args ...any) { log.Printf("ERROR: "+msg, args...) }

// Error logs to the warning log.
func Error(args ...any) { log.Print(append([]any{"ERROR:"}, args...)) }

// Fatalf logs to the error log and panics.
func Fatalf(msg string, args ...any) { log.Fatalf(msg, args...) }

// Fatal logs to the error log and panics.
func Fatal(args ...any) { log.Fatal(args...) }

// Fatalln logs to the error log and panics.
func Fatalln(args ...any) { log.Fatalln(args...) }

// Exit logs to the error log and exits.
func Exit(args ...any) { log.Fatal(args...) }

// Exitf logs to the error log and panics.
func Exitf(msg string, args ...any) { log.Fatalf(msg, args...) }

// InfoContext logs to the informational log with a Context.
func InfoContext(ctx context.Context, args ...any) {
	InfoContextf(ctx, defaultFormat(args), args...)
}

// WarningContext logs to the informational log with a Context.
func WarningContext(ctx context.Context, args ...any) {
	WarningContextf(ctx, defaultFormat(args), args...)
}

// ErrorContext logs to the informational log with a Context.
func ErrorContext(ctx context.Context, args ...any) {
	ErrorContextf(ctx, defaultFormat(args), args...)
}

// InfoContextf logs to the informational log with a Context.
func InfoContextf(ctx context.Context, msg string, args ...any) { Infof(msg, args...) }

// ErrorContextf logs to the error log with a Context.
func ErrorContextf(ctx context.Context, msg string, args ...any) { Errorf(msg, args...) }

// WarningContextf logs to the warning log with a Context.
func WarningContextf(ctx context.Context, msg string, args ...any) { Warningf(msg, args...) }

// defaultFormat returns a fmt.Printf format specifier that formats its
// arguments as if they were passed to fmt.Print.
func defaultFormat(args []any) string {
	n := len(args)
	switch n {
	case 0:
		return ""
	case 1:
		return "%v"
	}

	b := make([]byte, 0, n*3-1)
	wasString := true // Suppress leading space.
	for _, arg := range args {
		isString := arg != nil && reflect.TypeOf(arg).Kind() == reflect.String
		if wasString || isString {
			b = append(b, "%v"...)
		} else {
			b = append(b, " %v"...)
		}
		wasString = isString
	}
	return string(b)
}
