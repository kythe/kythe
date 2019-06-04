def cc_resources(name, data, strip = None):
    if strip:
        basename_expr = "$${j##*%s}" % (strip,)
    else:
        basename_expr = "$$(basename \"$${j}\")"
    out_inc = name + ".inc"
    cmd = ('echo "static const struct FileToc kPackedFiles[] = {" > $(@); \n' +
           "for j in $(SRCS); do\n" + (
               '  echo "{\\"%s\\"," >> $(@);\n' % (basename_expr,)
           ) +
           '  echo "R\\"filecontent($$(< $${j}))filecontent\\"" >> $(@);\n' +
           '  echo "}," >> $(@);\n' +
           "done &&\n" +
           'echo "{nullptr, nullptr}};" >> $(@)')
    if len(data) == 0:
        fail("Empty `data` attribute in `%s`" % name)
    native.genrule(
        name = name + "_inc",
        outs = [out_inc],
        srcs = data,
        cmd = cmd,
    )
    native.cc_library(
        name = name,
        hdrs = [name + "_inc"],
    )
