#!/usr/bin/env ruby
# encoding: UTF-8

symbols = {}

for line in IO.read(ARGV[0]).lines
  line.strip =~ /(fun|obj) (\w+): (.+)/
  symbols[$2] = {type: $1, versions: $3.split(', ')}
end

for sym in symbols.keys
  puts "#define SHIM_EXPORT_#{sym} \\"
  puts (symbols[sym][:versions].map do |version|
    sym_alias = "#{sym}_#{version.gsub('.', '_')}"
    [
      %Q! extern __typeof(shim_#{sym}) shim_#{sym_alias} __attribute__((alias("shim_#{sym}"))); !,
      %Q! __asm__(".symver shim_#{sym_alias},#{sym}@#{version}"); !
    ]
  end).flatten.join("\\\n")
  puts
end
