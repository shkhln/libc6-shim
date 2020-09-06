#!/usr/bin/env ruby
# encoding: UTF-8

require 'stringio'

LINUX = false

require(__dir__ + '/../src/prototypes.rb')
require(__dir__ + '/wrappers.rb')

CC = ENV['cc'] || 'cc'

START_TAG = "@@#{rand(1_000_000_000)}{{"
END_TAG   = "}}@@"

options, files = ARGV.partition{|arg| arg.start_with?('-')}

input = `#{CC} #{options.join(' ')} -E -DSHIM_SCAN -D"SHIM_WRAP(fun, ...)=#{START_TAG} fun @opts __VA_ARGS__ #{END_TAG}" #{files.join(' ')}`

input.scan(/#{START_TAG}\s([^\s]+)\s@opts(.*?)\s#{END_TAG}/m) do |function_name, options|

  raise "Unknown function #{function_name}" if not $functions[function_name]

  io = StringIO.new
  generate_wrapper(io, $functions[function_name], "shim_#{function_name}_impl")
  lines = io.string.lines

  puts "#define SHIM_WRAPPER_#{function_name} \\"
  puts lines.map{|line| line.gsub("\n", "\\\n")}
  puts "SHIM_EXPORT(#{function_name});"
  puts
end
