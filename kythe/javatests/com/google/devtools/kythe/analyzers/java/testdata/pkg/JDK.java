package com.google.devtools.kythe.analyzers.java.testdata.pkg;

// Check that we correctly detect classes provided by the JDK.

import org.xml.sax.SAXException;

public class JDK {

  static class SomeUserClass {};

  //- @s defines/binding S
  //- S typed vname(_,"jdk","","","java")
  String s;

  //- @se defines/binding SE
  //- SE typed vname(_,"jdk","","","java")
  SAXException se;

  //- @suc defines/binding SUC
  //- !{ SUC typed vname(_,"jdk","","","java") }
  SomeUserClass suc;
}
