package rdf

import "testing"

func q(s string) string { return `"` + s + `"` }

func TestQuote(t *testing.T) {
	tests := []struct {
		input, want string
	}{
		{"", q("")},                               // empty
		{"a b c", q("a b c")},                     // no escapes
		{"\x00", q(`\u0000`)},                     // NUL
		{"\x08\x09\x0a\x0c\x0d", q(`\b\t\n\f\r`)}, // C-style controls
		{`" \ '`, q(`\" \\ \'`)},                  // metacharacters
		{"§3.14 π", q(`\u00a73.14 \u03c0`)},       // non-ASCII (UTF-8)
		{"\xfe", q(`\u00fe`)},                     // non-UTF-8 single-byte
		{"\U0002A6D0", q(`\U0002a6d0`)},           // large UTF-8
	}
	for _, test := range tests {
		got := Quote(test.input)
		if got != test.want {
			t.Errorf("Quote %q: got %s, want %s", test.input, got, test.want)
		}
	}
}

func TestEncoding(t *testing.T) {
	tests := []struct {
		s, p, o string
		want    string
	}{
		{want: `"" "" "" .`}, // empty
		{"Mary", "loves", "cabbáge", `"Mary" "loves" "cabb\u00e1ge" .`},
		{"▷", "\xa7", `"`, `"\u25b7" "\u00a7" "\"" .`},
		{" ", "----", "\tq\r\n", `" " "----" "\tq\r\n" .`},
	}
	for _, test := range tests {
		triple := &Triple{test.s, test.p, test.o}
		got := triple.String()
		if got != test.want {
			t.Errorf("Encoding %+v\n got: %s\nwant: %s", triple, got, test.want)
		}
	}
}
