def cc_resources(name, data):
  out_inc = name + ".inc"
  cmd =('echo "static const struct FileToc kPackedFiles[] = {" > $(@); \n' +
        'for j in $(SRCS); do\n' +
        '  echo "{\\"$$(basename "$${j}")\\"," >> $(@);\n' +
        '  echo "R\\"filecontent($$(< $${j}))filecontent\\"" >> $(@);\n' +
        '  echo "}," >> $(@);\n' +
        'done &&\n' +
        'echo "{nullptr, nullptr}};" >> $(@)')
  native.genrule(
    name = name,
    outs = [out_inc],
    srcs = data,
    cmd = cmd
  )
