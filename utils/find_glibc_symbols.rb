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

      name    = $1
      version = $2
      type    =
        case line.split(/\s+/)[4]
          when 'FUNC'   then :fun
          when 'IFUNC'  then :fun
          when 'OS+0'   then :fun
          when 'OBJECT' then :obj
          else
            raise
        end

      if not symbols[name]
        symbols[name] = {versions: [], type: type}
      end

      if not symbols[name][:versions].include?(version)
        symbols[name][:versions] << version
      end
    end
  end
end

for sym in symbols.keys.sort
  print symbols[sym][:type], ' ', sym, ': ', symbols[sym][:versions].sort.join(', '), "\n"
end
