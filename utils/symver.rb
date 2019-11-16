#!/usr/bin/env ruby
# encoding: UTF-8

symbols = {}

for line in IO.read(ARGV[0]).lines
  sym, versions = line.strip.split(': ')
  symbols[sym] = versions.split(', ')
end

for sym in symbols.keys
  for version in symbols[sym]
    puts "__asm__(\".symver shim_#{sym},#{sym}@#{version}\");"
  end
end
