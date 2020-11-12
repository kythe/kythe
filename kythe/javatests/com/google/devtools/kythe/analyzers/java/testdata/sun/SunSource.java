package sun;

// Check that we correctly detect classes provided by the JDK,
// regardless of package name.

import org.xml.sax.SAXException;

public class SunSource {

  @SuppressWarnings("ClassCanBeStatic")
  class SomeUserClass {};

  //- @s defines/binding S
  //- S typed vname(_,"override","","","java")
  String s;

  //- @se defines/binding SE
  //- SE typed vname(_,"override","","","java")
  SAXException se;

  //- @suc defines/binding SUC
  //- !{ SUC typed vname(_,"override","","","java") }
  SomeUserClass suc;
}
