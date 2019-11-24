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
  fopen64:         'fopen',
  mkstemp64:       'mkstemp',
  open64:          'open',
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

def format_specifier(decl)

  return '...' if decl[:name] == '...'
  return '%p'  if decl[:type] =~ /^.+?\(\*\)\s*\([^\)]+\)$/

  case decl[:type]
    when /const char ?\*\*/
      '%p'
    when /const char ?\*[^\*]*/
      '%.100s'
    when /\*/, 'va_list'
      '%p'
    when 'int', 'uid_t', 'gid_t'
      '%d'
    when 'mode_t'
      '0%o'
    when 'long'
      '%ld'
    when 'unsigned long'
      '0x%lx'
    when 'socklen_t'
      '%u'
    when 'size_t'
      '%zu'
    when 'ssize_t'
      '%zd'
    when 'float', 'double'
      '%f'
    else
      '_'
  end
end

def log_args(args)

  log_fmt_parts = []
  log_args      = []

  for arg in args
    f = format_specifier(arg)
    log_fmt_parts << f
    log_args      << arg[:name] if f.include?('%')
  end

  fmt_string = '"%s(' + log_fmt_parts.map{|p| p =~ /%[^a-z]*s/ ? "\\\"#{p}\\\"" : p}.join(', ') + ')\\n"'

  if log_args.empty?
    "LOG(#{fmt_string}, __func__);"
  else
    "LOG(#{fmt_string}, __func__, #{log_args.join(', ')});"
  end
end

def log_result(decl)
  f = format_specifier(decl)
  'LOG("%s -> ' + (f =~ /%[^a-z]*s/ ? "\\\"#{f}\\\"" : f) + '\\n", __func__' + (f.include?('%') ? ', _ret_' : '') + ');'
end

def is_variadic(function)
  function[:args].size > 1 && function[:args].last[:name] == '...'
end

FUNCTION_POINTER_TYPE = /^(.+)?\(([\*\^])\)\s*(\([^\)]+\))$/

def generate_stub(function)
  puts '// ' + function[:prototype]
  puts 'void shim_' + function[:name] + '() {'
  puts '  UNIMPLEMENTED();'
  puts '}'
end

def generate_wrapper(function, shim_impl_exists)

  if function[:lsb] && !shim_impl_exists
    generate_stub(function)
    return
  end

  args = function[:args]
  args = [] if args.size == 1 && args.first[:type] == 'void'

  for type in args.map{|arg| arg[:type]} + [function[:type]]
    if type =~ /^(const |)struct (\w+)/
      struct = $2.to_sym

      if not STRUCT_COMPATIBILITY.keys.include?(struct)
        STDERR.puts "\e[31m#{$PROGRAM_NAME}: unknown struct #{struct}, skipping function #{function[:name]}\e[0m"
        generate_stub(function)
        return
      end

      compatible = STRUCT_COMPATIBILITY[struct]
      if !compatible && !shim_impl_exists
        STDERR.puts "\e[31m#{$PROGRAM_NAME}: found binary incompatible struct #{struct}, explicit shim impl required for function #{function[:name]}\e[0m"
        generate_stub(function)
        return
      end
    end
  end

  def to_decl(arg)
    case arg[:type]
      when /^(.+?)(\[(\d*|restrict)\])$/
        "#{$1} #{arg[:name]}#{$2}"
      when FUNCTION_POINTER_TYPE
        "#{$1}(#{$2}#{arg[:name]})#{$3}"
      else
        "#{arg[:type] ? arg[:type].gsub(/struct\s+/, 'linux_') : ''} #{arg[:name]}"
    end
  end

  for include in function[:includes]
    puts include
  end

  if shim_impl_exists
    puts function[:type] + ' shim_' + function[:name] + '_impl(' + function[:args].map(&method(:to_decl)).join(', ').gsub('...', 'va_list') + ');'
  end

  puts '// ' + function[:prototype]
  puts function[:type] + ' shim_' + function[:name] + '(' + function[:args].map(&method(:to_decl)).join(', ') + ') {'

  puts '  ' + log_args(args)

  if is_variadic(function)
    puts "  va_list _args_;"
    puts "  va_start(_args_, #{args[-2][:name]});"
  end

  if function[:type] != 'void'
    puts "  #{function[:type]} _ret_ = "
  end

  if shim_impl_exists
    print "  shim_#{function[:name]}_impl"
  else
    print '  '
    if is_variadic(function)
      print 'v'
    end
    print function[:name]
  end
  print '('
  print (args.map do |arg|
    if arg[:name] == '...'
      '_args_'
    else
      arg[:name]
    end
  end).join(', ')
  puts ');'

  if is_variadic(function)
    puts '  va_end(_args_);'
  end

  if function[:type] != 'void'
    puts '  ' + log_result(function)
    puts '  return _ret_;'
  end

  puts '}'
end

symbols = {}

for line in IO.read(ARGV[0]).lines
  line.strip =~ /(fun|obj) (\w+): (.+)/
  symbols[$2] = {type: $1, versions: $3.split(', ')}
end

implemented_wrappers = {}
implemented_shims    = {}

for line in `readelf -s #{ARGV[1]} | grep -v UND`.lines
  case line
    when /shim_([\w_]+)_impl/ then implemented_shims[$1]    = true
    when /shim_([\w_]+)/      then implemented_wrappers[$1] = true
  end
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

for sym in symbols.keys

  puts "// #{sym}"

  if symbols[sym][:type] == 'fun' && !SUBSTITUTIONS[sym.to_sym]

    if not implemented_wrappers[sym]
      implemented_wrappers[sym] = true

      function = $functions[sym]
      if function
        puts generate_wrapper(function, implemented_shims[function[:name]])
      else
        puts 'void shim_' + sym + '() {'
        puts '  UNIMPLEMENTED();'
        puts '}'
        puts
      end

      versions = symbols[sym][:versions]
      if versions.size > 1
        for version in versions
          puts "extern __typeof(shim_#{sym}) shim_#{sym}_#{version.gsub('.', '_')} __attribute__((alias(\"shim_#{sym}\")));"
        end
      end
    end
  end

  puts
end

puts

for sym, subst in SUBSTITUTIONS
  versions = symbols[sym.to_s][:versions]
  if versions.size == 1
    puts "extern __typeof(shim_#{subst}) shim_#{sym} __attribute__((alias(\"shim_#{subst}\")));"
  else
    for version in versions
      puts "extern __typeof(shim_#{subst}) shim_#{sym}_#{version.gsub('.', '_')} __attribute__((alias(\"shim_#{subst}\")));"
    end
  end
end

puts
