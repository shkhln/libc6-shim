#!/usr/bin/env ruby
# encoding: UTF-8

LINUX = true

require(__dir__ + '/prototypes.rb')

puts <<E
#define _GNU_SOURCE
#define _LARGEFILE_SOURCE 1  // ?
#define _FILE_OFFSET_BITS 64 // ?
E

for function in $functions.values
  puts function[:includes].join("\n")
  puts function[:prototype] + ';'
  puts
end
