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
	//- Flag named FlagName
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

	//- @featureFlag defines/binding FeatureFlagVar
	//- FeatureFlagVar denotes FeatureFlag
	//- FeatureFlag.node/kind google/gflag
	//- FeatureFlag.tag/deprecated ""
	featureFlag = flag.Int("feature", 42, "DEPRECATED: don't use this")
)

func init() {
	//- @localFlag defines/binding LocalFlag
	//- LocalFlag.node/kind variable
	//- @"\"bool_flag\"" defines/binding BoolFlag=vname("flag bool_flag", _, _, _, _)
	//- BoolFlag.node/kind google/gflag
	//- LocalFlag denotes BoolFlag
	localFlag := flag.Bool("bool_flag", false, "Bool flag")

	//- @localVar defines/binding LocalVar
	var localVar bool
	//- @"\"var_flag\"" defines/binding VarFlag=vname("flag var_flag", _, _, _, _)
	//- VarFlag.node/kind google/gflag
	//- VarFlagDoc documents VarFlag
	//- VarFlagDoc.node/kind doc
	//- VarFlagDoc.text "Var flag"
	//- LocalVar denotes VarFlag
	flag.BoolVar(&localVar, "var_flag", true, "Var flag")

	//- @"\"func_flag\"" defines/binding FuncFlag=vname("flag func_flag", _, _, _, _)
	//- FuncFlag.node/kind google/gflag
	//- FuncFlagDoc documents FuncFlag
	//- FuncFlagDoc.node/kind doc
	//- FuncFlagDoc.text "Func flag"
	flag.BoolFunc("func_flag", "Func flag", func(s string) error { return nil })

	fmt.Println(*flagVar, *localFlag) // use flag vars
}

func main() {
	flag.Parse()

	//- @"\"flag_name\"" ref FlagName=vname("flag_name", Corpus, "", "", "")
	//- FlagName.node/kind name
	f := flag.Lookup("flag_name")

	fmt.Println(flag.Args(), f)
}
