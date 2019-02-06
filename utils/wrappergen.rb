#!/usr/bin/env ruby
# encoding: UTF-8

require 'json'

IMPL_BLACKLIST = [
  'quick_exit',
  'iswctype_l',
  'rtime',

  'makecontext',
  'openat',
  'sem_open',
  'strfmon',
  'strfmon_l',
  'ulimit',

  'tmpnam',
  'tempnam',
  'gets',
  'mktemp',

  'cacoshl',
  'cacosl',
  'casinhl',
  'casinl',
  'catanhl',
  'catanl',

  'freelocale',
  'setgrent',

  'setjmp',
  '_setjmp',
  '__sigsetjmp'
]

SUBSTITUTIONS = {
  __ctype_toupper: 'toupper',
  __ctype_tolower: 'tolower',
  __fxstat64:      '__fxstat', # ?
  __getdelim:      'getdelim',
  __isoc99_sscanf: 'sscanf',
  __strdup:        'strdup',
  __strndup:       'strndup',
  _exit:           'exit',
  _IO_getc:        'getc',
  _IO_putc:        'putc',
  alphasort64:     'alphasort',
  ftello64:        'ftello',
  fseeko64:        'fseeko',
  fopen64:         'fopen',
  ftruncate64:     'ftruncate',
  lseek64:         'lseek',
  mkstemp64:       'mkstemp',
  mmap64:          'mmap',
  open64:          'open',
  pread64:         'pread',
  pwrite64:        'pwrite',
  readdir64:       'readdir',
  scandir64:       'scandir',
  statfs64:        'statfs',
  statvfs64:       'statvfs'
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
  utsname:     false
}

def format_specifier(decl)

  return '...' if decl['name'] == '...'
  return '%p'  if decl['type'] =~ /^.+?\(\*\)\s*\([^\)]+\)$/

  case decl['type']
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
    log_args      << arg['name'] if f.include?('%')
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
  function['args'].size > 1 && function['args'].last['name'] == '...'
end

FUNCTION_POINTER_TYPE = /^(.+)?\(([\*\^])\)\s*(\([^\)]+\))$/

def generate_stub(function)
  puts '// ' + function['prototype']
  puts 'void shim_' + function['name'] + '() {'
  puts '  UNIMPLEMENTED();'
  puts '}'
end

def generate_wrapper(function, shim_impl_exists)

  if function[:lsb] && !shim_impl_exists
    generate_stub(function)
    return
  end

  args = function['args']
  args = [] if args.size == 1 && args.first['type'] == 'void'

  for type in args.map{|arg| arg['type']} + [function['type']]
    if type =~ /^(const |)struct (\w+)/
      struct = $2.to_sym

      if not STRUCT_COMPATIBILITY.keys.include?(struct)
        STDERR.puts "\e[31m#{$PROGRAM_NAME}: unknown struct #{struct}, skipping function #{function['name']}\e[0m"
        generate_stub(function)
        return
      end

      compatible = STRUCT_COMPATIBILITY[struct]
      if !compatible && !shim_impl_exists
        STDERR.puts "\e[31m#{$PROGRAM_NAME}: found binary incompatible struct #{struct}, explicit shim impl required for function #{function['name']}\e[0m"
        generate_stub(function)
        return
      end
    end
  end

  def to_decl(arg)
    case arg['type']
      when /^(.+?)(\[(\d*|restrict)\])$/
        "#{$1} #{arg['name']}#{$2}"
      when FUNCTION_POINTER_TYPE
        "#{$1}(#{$2}#{arg['name']})#{$3}"
      else
      "#{arg['type']} #{arg['name']}"
    end
  end

  for include in function['includes']
    puts include
  end

  if shim_impl_exists
    puts function['type'] + ' shim_' + function['name'] + '_impl(' + function['args'].map(&method(:to_decl)).join(', ').gsub('...', 'va_list') + ');'
  end

  puts '// ' + function['prototype']
  puts function['type'] + ' shim_' + function['name'] + '(' + function['args'].map(&method(:to_decl)).join(', ') + ') {'

  puts '  ' + log_args(args)

  if is_variadic(function)
    puts "  va_list _args_;"
    puts "  va_start(_args_, #{args[-2]['name']});"
  end

  if function['type'] != 'void'
    puts "  #{function['type']} _ret_ = "
  end

  if shim_impl_exists
    print "  shim_#{function['name']}_impl"
  else
    print '  '
    if is_variadic(function)
      print 'v'
    end
    print function['name']
  end
  print '('
  print (args.map do |arg|
    if arg['name'] == '...'
      '_args_'
    else
      arg['name']
    end
  end).join(', ')
  puts ');'

  if is_variadic(function)
    puts '  va_end(_args_);'
  end

  if function['type'] != 'void'
    puts '  ' + log_result(function)
    puts '  return _ret_;'
  end

  puts '}'
end

functions = {}

for _, synopsis in JSON.parse(IO.read(__dir__ + '/../bsd-functions.json', {mode: 'r:UTF-8'}))
  for function in synopsis['functions']
    functions[function['name']] = function
  end
end

LINUX_INCLUDES = [
  '#include <argz.h>',
  '#include <envz.h>',
  '#include <error.h>',
  '#include <utmp.h>',
  '#include <gnu/libc-version.h>',
  '#include <sys/epoll.h>',
  '#include <sys/sendfile.h>',
  '#include <sys/statfs.h>'
]

for _, synopsis in JSON.parse(IO.read(__dir__ + '/../lsb-functions.json', {mode: 'r:UTF-8'}))
  for function in synopsis['functions']

    next if functions[function['name']]

    functions[function['name']] = {
      'prototype' => function['prototype'],
      'name'      => function['name'],
      'type'      => function['type'],
      'args'      => function['args'],
      'includes'  => function['includes'].find_all{|str| !LINUX_INCLUDES.include?(str)},
      :lsb        => true
    }
  end
end

symbols = {}

for line in IO.read(ARGV[0]).lines
  sym, versions = line.strip.split(': ')
  symbols[sym] = versions.split(', ')
end

implemented_wrappers = {}
implemented_shims    = {}

for line in `readelf -s #{ARGV[1]} | grep -v UND`.lines
  case line
    when /shim_([\w_]+)_impl/ then implemented_shims[$1]    = true
    when /shim_([\w_]+)/      then implemented_wrappers[$1] = true
  end
end

puts '#define _WITH_GETLINE'
puts '#include <stdarg.h>'
puts '#include <stdint.h>'
puts '#include "../src/shim.h"'
puts

puts 'struct stat64 {};'
puts 'typedef void* __dispatch_fn_t;'
puts 'typedef void _IO_FILE;'
puts 'typedef void cpu_set_t;'
puts 'typedef void glob64_t;'
puts 'typedef int error_t;'

for sym in symbols.keys

  function = functions[SUBSTITUTIONS[sym.to_sym] || sym]
  if function && !(IMPL_BLACKLIST.include?(function['name']) || function['name'] =~ /iconv/)

    for version in symbols[sym]
      puts "__asm__(\".symver shim_#{function['name']},#{sym}@#{version}\");"
    end

    if not implemented_wrappers[function['name']]
      implemented_wrappers[function['name']] = true
      puts generate_wrapper(function, implemented_shims[function['name']])
    end

    puts
  end
end
