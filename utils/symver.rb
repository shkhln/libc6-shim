#!/usr/bin/env ruby
# encoding: UTF-8

symbols = {}

for line in IO.read(ARGV[0]).lines
  line.strip =~ /(fun|obj) (\w+): (.+)/
  symbols[$2] = {type: $1, versions: $3.split(', ')}
end

for sym in symbols.keys
  versions = symbols[sym][:versions]
  if versions.size == 1
    puts "__asm__(\".symver shim_#{sym},#{sym}@#{versions.first}\");"
  else
    for version in versions
      puts "__asm__(\".symver shim_#{sym}_#{version.gsub('.', '_')},#{sym}@#{version}\");"
    end
  end
end
