#!/usr/bin/env ruby
# encoding: utf-8

require 'json'
require(__dir__ + '/_prototypes.rb')

PROTOTYPE_BLACKLIST = [
  /__libc_start_main/,
  /_pthread_cleanup_pop/,
  /_pthread_cleanup_push/
]

def indent_level(line)
  res = 0
  for c in line.chars
    break if c != ' '
    res += 1
  end
  res
end

json = {}
spec = IO.read(__dir__ + '/LSB-Core-generic.txt')
  .lines.map(&:chop).map{|line| line.chars.collect { |c| c.valid_encoding? ? c : '?' }.join}.to_a

for header in [
  '        14.5. Interface Definitions for libc',
  '        14.8. Interface Definitions for libm',
  '        14.11. Interface Definitions for libpthread',
  '        14.17. Interface Definitions for libdl'
]

  functions = []

  for i in (spec.index(header) + 1)...(spec.length)
    line = spec[i]
    break if line != '' && indent_level(line) == 0
    functions << $1 if line =~ /([\w\d_]+)\s--/i
  end

  header.strip!

  json[header] = {
    functions: []
  }

  for f in functions
    section  = nil
    synopsis = ''

    fi = spec.index(f)
    if fi
      for i in (fi + 1)...(spec.length)
        line = spec[i]
        case line
          when 'Name', 'Synopsis', 'Description'
            section = line
          when /^([\w\d_]+)$/i
            break
          else
            if section == 'Synopsis'
              synopsis += line
              synopsis += "\n"
            end
        end
      end
      json[header][:functions] += parse_synopsis(synopsis)
    else
      STDERR.puts "#{f} description not found"
    end
  end
end

puts JSON.pretty_generate(json)
