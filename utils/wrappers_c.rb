#!/usr/bin/env ruby
# encoding: UTF-8

SUBSTITUTIONS = {
  __getdelim:      'getdelim',
  __isoc99_sscanf: 'sscanf',
  __libc_free:     'free',
  __libc_malloc:   'malloc',
  __strdup:        'strdup',
  __strndup:       'strndup',
  _exit:           '_Exit',
  _IO_getc:        'getc',
  _IO_putc:        'putc',
  creat64:         'creat',
  mkstemp64:       'mkstemp'
}

STRUCT_COMPATIBILITY = {
  FTW:          true,
  addrinfo:     false,
  cmsghdr:      false, # compatible on i386
  dirent:       false,
  dl_phdr_info: true,
  in_addr:      true,
  iovec:        true,
  hostent:      true,
  msghdr:       false, # compatible on i386
  option:       true,
  passwd:       false,
  pollfd:       true,
  protoent:     true,
  rlimit:       false, # compatible on x86_64
  rusage:       true,
  sembuf:       true,
  sched_param:  true,
  shmid_ds:     false,
  sigaction:    false,
  sockaddr:     false,
  stat:         false,
  statfs:       false,
  termios:      false,
  timespec:     true,
  timeval:      true,
  timezone:     true,
  tm:           true,
  utsname:      false,
  stat64:       false,
  statfs64:     false
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
#include "../src/libc/locale.h"
#include "../src/libc/netdb.h"
#include "../src/libc/signal.h"
#include "../src/libc/sched.h"
#include "../src/libc/time.h"
#include "../src/libc/sys/mount.h"
#include "../src/libc/sys/socket.h"
#include "../src/libc/sys/stat.h"
#include "../src/libc/sys/utsname.h"
#include "../src/libthr/pthread.h"

typedef struct protoent linux_protoent;

E

LINUX = false

require(__dir__ + '/../src/prototypes.rb')
require(__dir__ + '/wrappers.rb')

def check_compat(function)

  args = function[:args]
  args = [] if args.size == 1 && args.first[:type] == 'void'

  for type in args.map{|arg| arg[:type]} + [function[:type]]
    case type
      when /^(const |)pthread_(barrier|cond|mutex)attr_t\*/
        return false
      when /ucontext_t/
        return false
      when /^(const |)struct (\w+)/

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

  if symbols[sym][:type] == 'fun' && !(SUBSTITUTIONS[sym.to_sym] || sym.to_sym =~ /setjmp|__libc_start_main/)

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
  puts "SHIM_EXPORT(#{sym});"
end

puts
