// Binary formatjson reads json from stdin and writes formatted json to stdout.
package main

import (
	"encoding/json"
	"log"
	"os"
)

func main() {
	obj := make(map[string]interface{})
	d := json.NewDecoder(os.Stdin)
	if err := d.Decode(&obj); err != nil {
		log.Fatal(err)
	}

	e := json.NewEncoder(os.Stdout)
	e.SetIndent("", "    ")
	if err := e.Encode(obj); err != nil {
		log.Fatal(err)
	}
}
