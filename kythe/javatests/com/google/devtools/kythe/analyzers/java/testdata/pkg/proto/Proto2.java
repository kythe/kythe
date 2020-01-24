package pkg.proto2;

import static pkg.proto2.Testdata2.ExtensionMessage;
import static pkg.proto2.Testdata2.OuterMessage;

//- vname("",_, "", "kythe/javatests/com/google/devtools/kythe/analyzers/java/testdata/pkg/proto/testdata2.proto","") generates vname("", _, "bazel-out/bin", "kythe/javatests/com/google/devtools/kythe/analyzers/java/testdata/pkg/proto/proto2_gensrc/pkg/proto2/Txestdata2.java", "")
public class Proto2 {
    public static void main() {
        //- @OuterMessage ref JavaOuterMessage
        OuterMessage msg =
            OuterMessage.newBuilder().build();
        //- @ExtensionMessage ref JavaExtensionMessage
        //- @extension ref JavaExtensionField
        msg.hasExtension(ExtensionMessage.extension);
    }

    //- OuterMessage generates JavaOuterMessage

    //- ExtensionMessage generates JavaExtensionMessage
    //- ExtensionField generates JavaExtensionField
}
