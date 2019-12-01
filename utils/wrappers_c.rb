#!/usr/bin/env ruby
# encoding: UTF-8

SUBSTITUTIONS = {
  __ctype_toupper: 'toupper',
  __ctype_tolower: 'tolower',
  __getdelim:      'getdelim',
  __isoc99_sscanf: 'sscanf',
  __strdup:        'strdup',
  __strndup:       'strndup',
  _exit:           '_Exit',
  _IO_getc:        'getc',
  _IO_putc:        'putc',
  mkstemp64:       'mkstemp'
}

STRUCT_COMPATIBILITY = {
  cmsghdr:     false, # compatible on i386
  dirent:      false,
  in_addr:     true,
  iovec:       true,
  msghdr:      false, # compatible on i386
  option:      true,
  pollfd:      true,
  rlimit:      false, # compatible on x86_64
  rusage:      true,
  sembuf:      true,
  sched_param: true,
  shmid_ds:    false,
  sockaddr:    false,
  stat:        false,
  statfs:      false,
  tm:          true,
  timespec:    true,
  timeval:     true,
  timezone:    true,
  utsname:     false,
  stat64:      false,
  statfs64:    false
}

symbols = {}

for line in IO.read(ARGV[0]).lines
  line.strip =~ /(fun|obj) (\w+): (.+)/
  symbols[$2] = {type: $1, versions: $3.split(', ')}
end

puts <<E
#include <stdarg.h>
#include <stdint.h>

#include "../src/shim.h"
#include "../src/libc/dirent.h"
#include "../src/libc/time.h"
#include "../src/libc/sys/mount.h"
#include "../src/libc/sys/socket.h"
#include "../src/libc/sys/stat.h"
#include "../src/libc/sys/utsname.h"

E

LINUX = false

require(__dir__ + '/prototypes.rb')
require(__dir__ + '/wrappers.rb')

def check_compat(function)

  args = function[:args]
  args = [] if args.size == 1 && args.first[:type] == 'void'

  for type in args.map{|arg| arg[:type]} + [function[:type]]
    if type =~ /^(const |)struct (\w+)/

      struct = $2.to_sym

      if not STRUCT_COMPATIBILITY.keys.include?(struct)
        STDERR.puts "\e[31m#{$PROGRAM_NAME}: unknown struct #{struct}, skipping function #{function[:name]}\e[0m"
        return false
      end

      compatible = STRUCT_COMPATIBILITY[struct]
      if !compatible
        #~ STDERR.puts "\e[31m#{$PROGRAM_NAME}: found binary incompatible struct #{struct}, explicit shim impl required for function #{function[:name]}\e[0m"
        return false
      end
    end
  end

  true
end

def generate_stub(name)
  puts 'void shim_' + name + '() {'
  puts '  UNIMPLEMENTED();'
  puts '}'
end

for sym in symbols.keys

  puts "// #{sym}"

  if symbols[sym][:type] == 'fun' && !SUBSTITUTIONS[sym.to_sym]

    puts "#ifndef SHIM_WRAPPER_#{sym}"
    puts

    function = $functions[sym]
    if function
      puts '// ' + function[:prototype]
      if !function[:lsb] && check_compat(function)
        for include in function[:includes]
          puts include
        end
        generate_wrapper(STDOUT, function, nil)
      else
        generate_stub(function[:name])
      end      
    else
      generate_stub(sym)
    end
    puts

    puts "SHIM_EXPORT(#{sym});"
    puts

    puts "#endif // SHIM_WRAPPER_#{sym}"
  end

  puts
end

puts

for sym, subst in SUBSTITUTIONS
  puts "extern __typeof(shim_#{subst}) shim_#{sym} __attribute__((alias(\"shim_#{subst}\")));"
  puts "SHIM_EXPORT(#{sym})";
end

puts
