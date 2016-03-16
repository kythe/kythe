package shell

import (
	"fmt"
	"io/ioutil"
	"log"
	"reflect"
	"strings"
	"testing"
)

func TestQuote(t *testing.T) {
	tests := []struct {
		in, want string
	}{
		{"", "''"},            // empty is special
		{"abc", "abc"},        // nothing to quote
		{"--flag", "--flag"},  // "
		{"'abc", `\'abc`},     // single quote only
		{"abc'", `abc\'`},     // "
		{`shan't`, `shan\'t`}, // "
		{"--flag=value", `'--flag=value'`},
		{"a b\tc", "'a b\tc'"},
		{`a"b"c`, `'a"b"c'`},
		{`'''`, `\'\'\'`},
		{`\`, `'\'`},
		{`'a=b`, `\''a=b'`},   // quotes and other stuff
		{`a='b`, `'a='\''b'`}, // "
		{`a=b'`, `'a=b'\'`},   // "
	}
	for _, test := range tests {
		got := Quote(test.in)
		if got != test.want {
			t.Errorf("Quote %q: got %q, want %q", test.in, got, test.want)
		}
	}
}

func TestSplit(t *testing.T) {
	tests := []struct {
		in   string
		want []string
		ok   bool
	}{
		// Variations of empty input yield an empty split.
		{"", nil, true},
		{"   ", nil, true},
		{"\t", nil, true},
		{"\n ", nil, true},

		// Various escape sequences work properly.
		{`\ `, []string{" "}, true},
		{`a\ `, []string{"a "}, true},
		{`\\a`, []string{`\a`}, true},
		{`"a\"b"`, []string{`a"b`}, true},
		{`'\'`, []string{"\\"}, true},

		// Leading and trailing whitespace are discarded correctly.
		{"a", []string{"a"}, true},
		{" a", []string{"a"}, true},
		{"a\n", []string{"a"}, true},

		// Escaped newlines are magic in the correct ways.
		{"a\\\nb", []string{"ab"}, true},
		{"a \\\n  b\tc", []string{"a", "b", "c"}, true},

		// Various splits with and without quotes.  Quoted whitespace is
		// preserved.
		{"a b c", []string{"a", "b", "c"}, true},
		{`a 'b c'`, []string{"a", "b c"}, true},
		{"\"a\nb\"cd e'f'", []string{"a\nbcd", "ef"}, true},
		{"'\n \t '", []string{"\n \t "}, true},

		// Quoted empty strings are preserved in various places.
		{"''", []string{""}, true},
		{"a ''", []string{"a", ""}, true},
		{" a \"\" b ", []string{"a", "", "b"}, true},
		{"'' a", []string{"", "a"}, true},

		// Unbalanced quotation marks and escapes are detected.
		{"\\", []string{""}, false},          // escape without a target
		{"'", []string{""}, false},           // unclosed single
		{`"`, []string{""}, false},           // unclosed double
		{`'\''`, []string{`\`}, false},       // unclosed connected double
		{`"\\" '`, []string{`\`, ``}, false}, // unclosed separate single
		{"a 'b c", []string{"a", "b c"}, false},
		{`a "b c`, []string{"a", "b c"}, false},
		{`a "b \"`, []string{"a", `b "`}, false},
	}
	for _, test := range tests {
		got, ok := Split(test.in)
		if ok != test.ok {
			t.Errorf("Split %#q: got valid=%v, want %v", test.in, ok, test.ok)
		}
		if !reflect.DeepEqual(got, test.want) {
			t.Errorf("Split %#q: got %+q, want %+q", test.in, got, test.want)
		}
	}
}

func TestRoundTrip(t *testing.T) {
	tests := [][]string{
		nil,
		{"a"},
		{"a "},
		{"a", "b", "c"},
		{"a", "b c"},
		{"--flag=value"},
		{"m='$USER'", "nop+", "$$"},
		{`"a" b `, "c"},
		{"odd's", "bodkins", "x'", "x''", "x\"\"", "$x':y"},
		{"a=b", "--foo", "${bar}", `\$`},
		{"cat", "a${b}.txt", "|", "tee", "capture", "2>", "/dev/null"},
	}
	for _, test := range tests {
		s := Join(test)
		t.Logf("Join %#q = %v", test, s)
		got, ok := Split(s)
		if !ok {
			t.Errorf("Split %+q: should be valid, but is not", s)
		}
		if !reflect.DeepEqual(got, test) {
			t.Errorf("Split %+q: got %q, want %q", s, got, test)
		}
	}
}

func ExampleScanner() {
	const input = `a "free range" exploration of soi\ disant novelties`
	s := NewScanner(strings.NewReader(input))
	sum, count := 0, 0
	for s.Next() {
		count++
		sum += len(s.Text())
	}
	fmt.Println(len(input), count, sum, s.Complete(), s.Err())
	// Output: 51 6 43 true EOF
}

func ExampleScanner_Rest() {
	const input = `things 'and stuff' %end% all the remaining stuff`
	s := NewScanner(strings.NewReader(input))
	for s.Next() {
		if s.Text() == "%end%" {
			fmt.Print("found marker; ")
			break
		}
	}
	rest, err := ioutil.ReadAll(s.Rest())
	if err != nil {
		log.Fatal(err)
	}
	fmt.Println(string(rest))
	// Output: found marker; all the remaining stuff
}

func ExampleScanner_Each() {
	const input = `a\ b 'c d' "e f's g" stop "go directly to jail"`
	if err := NewScanner(strings.NewReader(input)).Each(func(tok string) bool {
		fmt.Println(tok)
		return tok != "stop"
	}); err != nil {
		log.Fatal(err)
	}
	// Output:
	// a b
	// c d
	// e f's g
	// stop
}
