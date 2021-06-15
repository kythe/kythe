#!/usr/bin/ruby
require 'asciidoctor'

input = Asciidoctor.load_file(ARGV[0])
File.open(ARGV[1], "w") { |out|
  out.puts "---"
  out.puts "layout: page"
  out.puts ["title: ", input.doctitle].join
  out.puts ["priority: ", input.attributes["priority"]].join
  out.puts ["toclevels: ", input.attributes["toclevels"]].join
  if input.attributes["toc"] != nil || input.attributes["toc2"] != nil
    out.puts "toc: true"
  end
  out.puts "---"
}
