#!/usr/bin/env ruby
# encoding: UTF-8

require 'json'

SUBSTITUTIONS = {
  __ctype_toupper: 'toupper',
  __ctype_tolower: 'tolower',
  __getdelim:      'getdelim',
  __sigsetjmp:     'sigsetjmp',
  __strdup:        'strdup',
  __strndup:       'strndup',
  _exit:           'exit',
  _IO_getc:        'getc',
  _IO_putc:        'putc',
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
  statfs64:        'statfs'
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

def generate_wrapper(function, impl_exists)

  args = function['args']
  args = [] if args.size == 1 && args.first['type'] == 'void'

  for arg in args
    if arg['type'] =~ /^(:?const )struct (\w+)/
      struct = $2.to_sym

      if not STRUCT_COMPATIBILITY.keys.include?(struct)
        STDERR.puts "\e[31m#{$PROGRAM_NAME}: unknown struct #{struct}, skipping function #{function['name']}\e[0m"
        return
      end

      compatible = STRUCT_COMPATIBILITY[struct]
      if !compatible && !impl_exists
        STDERR.puts "\e[31m#{$PROGRAM_NAME}: found binary incompatible struct #{struct}, explicit shim impl required for function #{function['name']}\e[0m"
        return
      end
    end
  end

  puts function['prototype'].sub(function['name'] + '(', 'shim_' + function['name'] + '(') + ' {'

  puts '  ' + log_args(args)

  if is_variadic(function)
    puts "  va_list _args_;"
    puts "  va_start(_args_, #{args[-2]['name']});"
  end

  if function['type'] != 'void'
    puts "  #{function['type']} _ret_ = "
  end

  if impl_exists
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

for _, synopsis in JSON.parse(IO.read(__dir__ + '/../functions.json', {mode: 'r:UTF-8'}))
  for function in synopsis['functions']
    functions[function['name']] = function
  end
end

required_symbols = {}

for line in IO.read(ARGV[0]).lines
  sym, versions = line.strip.split(': ')
  required_symbols[sym] = versions.split(', ')
end

includes = []

for sym, _ in required_symbols
  function = functions[sym]
  includes << function['includes'] if function
end

puts '#include <stdarg.h>'

for include in includes.uniq
  puts include
end

puts '#include "../src/shim.h"'
puts

implemented_wrappers = {}
implemented_shims    = {}

for line in `readelf -s #{ARGV[1]} | grep -v UND`.lines
  case line
    when /shim_(\w+)_impl/ then implemented_shims[$1]    = true
    when /shim_(\w+)/      then implemented_wrappers[$1] = true
  end
end

for fun in implemented_shims.keys
  function = functions[fun]
  if function
    puts function['prototype'].gsub(fun, 'shim_' + fun + '_impl').gsub('...', 'va_list') + ';'
  end
end

puts

for sym in required_symbols.keys

  function = functions[SUBSTITUTIONS[sym.to_sym] || sym]
  if function

    puts '/* ' + function['prototype'] + ' */'

    for version in required_symbols[sym]
      puts "__asm__(\".symver shim_#{function['name']},#{sym}@#{version}\");"
    end

    if not implemented_wrappers[function['name']]
      implemented_wrappers[function['name']] = true
      puts generate_wrapper(function, implemented_shims[function['name']])
    end

    puts
  end
end
