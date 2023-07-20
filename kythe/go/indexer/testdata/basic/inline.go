package subject

var Primary = struct {
	MyParam bool
}{
	MyParam: true,
}

//- EA.node/kind anchor
//- EA.loc/start 21
//- EA.loc/end 28
//- EA defines/binding Primary
//- vname("", default, "", "test/example.txt", "") generates vname("", kythe, "", "go/indexer/inline_test/inline.go", "")
//- _Alt=vname("IDENTIFIER:Primary", default, "", "test/example.txt", lang) generates Primary
//- Primary.node/kind variable

//- EB.node/kind anchor
//- EB.loc/start 41
//- EB.loc/end 48
//- EB defines/binding MyParam
//- _Alt2=vname("IDENTIFIER:Primary.my_param", default, "", "test/example.txt", lang) generates MyParam
//- MyParam.node/kind variable

//gokythe-inline-metadata:ElMSFiUva3l0aGUvZWRnZS9nZW5lcmF0ZXMaNQoSSURFTlRJRklFUjpQcmltYXJ5EgdkZWZhdWx0IhB0ZXN0L2V4YW1wbGUudHh0KgRsYW5nIBUoHBJcEhYlL2t5dGhlL2VkZ2UvZ2VuZXJhdGVzGj4KG0lERU5USUZJRVI6UHJpbWFyeS5teV9wYXJhbRIHZGVmYXVsdCIQdGVzdC9leGFtcGxlLnR4dCoEbGFuZyApKDA=
