#!/usr/bin/env ruby
# encoding: utf-8

symbols = {}

libs = Dir.glob(ARGV[0] + '/**/*.so*')

for lib in libs
  for line in `readelf -s #{lib}`.lines
    if line =~ /UND\s(\w+)@(GLIBC_[0-9\.]+)/i
      symbols[$1] =  [] if not symbols[$1]
      symbols[$1] << $2 unless symbols[$1].include?($2)
    end
  end
end

for sym in symbols.keys.sort
  print sym, ': ', symbols[sym].sort.join(', '), "\n"
end
