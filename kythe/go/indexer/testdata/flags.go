package flags

import (
	"flag"
	"fmt"
)

//- vname(_, Corpus, Root, Path, _).node/kind package

func CustomFlag(name string, def struct{}, description string) *struct{} { return &def }

var (
	//- @flagVar defines/binding FlagVar
	//- FlagVar.node/kind variable
	//- @"\"flag_name\"" defines/binding Flag=vname("flag flag_name", Corpus, Root, Path, _)
	//- Flag.node/kind google/gflag
	//- FlagVar denotes Flag
	flagVar = flag.String("flag_name", "default_value", "Flag description")

	//- FlagDoc documents Flag
	//- FlagDoc.node/kind doc
	//- FlagDoc.text "Flag description"

	//- @customFlag defines/binding CustomFlagVar
	//- @"\"custom_flag\"" defines/binding CustomFlag=vname("flag custom_flag", Corpus, Root, Path, _)
	//- CustomFlag.node/kind google/gflag
	//- CustomFlagVar denotes CustomFlag
	customFlag = CustomFlag("custom_flag", struct{}{}, "A custom flag desc")
)

func init() {
	//- @localFlag defines/binding LocalFlag
	//- LocalFlag.node/kind variable
	//- @"\"bool_flag\"" defines/binding BoolFlag=vname("flag bool_flag", _, _, _, _)
	//- BoolFlag.node/kind google/gflag
	//- LocalFlag denotes BoolFlag
	localFlag := flag.Bool("bool_flag", false, "Bool flag")

	args := flag.Args()
	fmt.Println(*flagVar, *localFlag, args)
}
