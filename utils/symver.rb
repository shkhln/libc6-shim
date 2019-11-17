#!/usr/bin/env ruby
# encoding: UTF-8

symbols = {}

for line in IO.read(ARGV[0]).lines
  line.strip =~ /(fun|obj) (\w+): (.+)/
  symbols[$2] = {type: $1, versions: $3.split(', ')}
end

for sym in symbols.keys
  for version in symbols[sym][:versions]
    puts "__asm__(\".symver shim_#{sym},#{sym}@#{version}\");"
  end
end
