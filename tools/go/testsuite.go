// Package testsuite contains types for a simplified version of Bazel's XML test
// results output file format.
package testsuite

import "encoding/xml"

// TestError represents an error result from a test.
type TestError struct {
	Message string `xml:"message,attr,omitempty"`
	Content string `xml:",chardata"`
}

// Status is summary whether a test case ran or not.
type Status int

// Test status kinds; whether a test ran.
const (
	NotRun Status = iota + 1
	Run
)

// MarshalXMLAttr implements the xml.MarshalAttr interface.
func (s Status) MarshalXMLAttr(name xml.Name) (xml.Attr, error) {
	var val string
	switch s {
	case NotRun:
		val = "notrun"
	case Run:
		val = "run"
	}
	return xml.Attr{
		Name:  xml.Name{Local: "status"},
		Value: val,
	}, nil
}

// TestCase is a single named test run with 0 or more errors.
type TestCase struct {
	Name   string  `xml:"name,attr,omitempty"`
	Time   float64 `xml:"time,attr,omitempty"`
	Status Status  `xml:"status,attr,omitempty"`

	Errors []TestError `xml:"error"`
}

// Suite is a named set of test cases.
type Suite struct {
	Name     string     `xml:"name,attr,omitempty"`
	TestCase []TestCase `xml:"testcase"`
}
