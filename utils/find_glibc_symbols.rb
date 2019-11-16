#!/usr/bin/env ruby
# encoding: utf-8

symbols = {}

libs = []
for lib in ['libc.so.6', 'libm.so.6', 'libdl.so.2', 'librt.so.1', 'libpthread.so.0']
  libs << (ARGV.include?('-32') ? "/compat/linux/lib/#{lib}" : "/compat/linux/lib64/#{lib}")
end

for lib in libs
  for line in `readelf -s #{lib}`.lines
    if !(line =~ /(UND|LOCAL)/) && line =~ /\s(\w+)@@?(GLIBC_[0-9\.]+)/i
      symbols[$1] =  [] if not symbols[$1]
      symbols[$1] << $2 unless symbols[$1].include?($2)
    end
  end
end

for sym in symbols.keys.sort
  print sym, ': ', symbols[sym].sort.join(', '), "\n"
end
