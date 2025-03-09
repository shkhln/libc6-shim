# encoding: utf-8

def format_specifier(decl)

  return '...' if decl[:name] == '...'
  return '%p [callback]' if decl[:type] =~ /^.+?\(\*\)\s*\([^\)]+\)$/

  case decl[:type]
    when /const char ?\*\*/
      '%p'
    when /const char ?\*[^\*]*/
      '%.100s'
    when /\*/, 'va_list'
      '%p'
    when 'int', 'uid_t', 'gid_t', 'nl_item'
      '%d'
    when 'mode_t'
      '0%o'
    when 'long'
      '%ld'
    when 'unsigned long'
      '0x%lx'
    when 'unsigned int', 'socklen_t'
      '%u'
    when 'size_t'
      '%zu'
    when 'ssize_t'
      '%zd'
    when 'float', 'double'
      '%f'
    when 'off_t'
      '%zd'
    when 'off64_t'
      '%jd'
    when 'iconv_t', 'locale_t', 'pthread_t'
      '%p'
    when 'time_t'
      '%zd'
    when 'clockid_t', 'pid_t', 'wchar_t'
      '%d'
    when 'pthread_key_t', 'wint_t'
      '%u'
    when 'wctype_t'
      '%lu'
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

  if log_fmt_parts.empty?
    "LOG_ENTRY();"
  else
    fmt_string = log_fmt_parts.map{|p| p =~ /%[^a-z]*s/ ? '\"' + p + '\"' : p}.join(', ')
    if log_args.empty?
      "LOG_ENTRY(\"#{fmt_string}\");"
    else
      "LOG_ENTRY(\"#{fmt_string}\", #{log_args.join(', ')});"
    end
  end
end

def log_result(function)
  if function[:type] == 'void'
    "LOG_EXIT();"
  else
    f = format_specifier(function)
    "LOG_EXIT(\"#{f =~ /%[^a-z]*s/ ? '\"' + f + '\"' : f}\"#{f.include?('%') ? ', _ret_' : ''});"
  end
end

def is_variadic(function)
  function[:args].size > 1 && function[:args].last[:name] == '...'
end

FUNCTION_POINTER_TYPE = /^(.+)?\(([\*\^])\)\s*(\([^\)]+\))$/

def to_shim_type(type)
  case type
    when 'cpu_set_t*'
      'linux_cpu_set_t*'
    when 'mode_t'
      'linux_mode_t'
    when 'off_t'
      'linux_off_t'
    when 'off64_t'
      'linux_off64_t'
    when 'fpos_t*'
      'linux_fpos_t*'
    when 'fpos64_t*'
      'linux_fpos64_t*'
    when 'DIR*'
      'linux_DIR*'
    when 'locale_t'
      'linux_locale_t'
    when 'pthread_once_t*'
      'linux_pthread_once_t*'
    when 'pthread_mutex_t*'
      'linux_pthread_mutex_t*'
    when /^(const |)pthread_(barrier|cond|mutex|rwlock)attr_t\*/
      $1 + 'linux_pthread_' + $2 + 'attr_t*'
    when /(^|\s)(stack_t|sigset_t)($|[^\w_])/
      type.gsub(/(^|\s)(stack_t|sigset_t)($|[^\w_])/) { "#{$1}linux_#{$2}#{$3}" }
    else
      type.gsub(/struct\s+/, 'linux_')
  end
end

def generate_wrapper(out, function, shim_fun_impl)

  args = function[:args]
  args = [] if args.size == 1 && args.first[:type] == 'void'

  def to_decl(arg)
    case arg[:type]
      when /^(.+?)\s?(\[(\d*|restrict)\])$/
        "#{$1} #{arg[:name]}#{$2}"
      when FUNCTION_POINTER_TYPE
        "#{to_shim_type($1)}(#{$2}#{arg[:name]})#{to_shim_type($3)}"
      else
        "#{arg[:type] ? to_shim_type(arg[:type]) : ''} #{arg[:name]}"
    end
  end

  out.puts to_shim_type(function[:type]) + ' shim_' + function[:name] + '(' + function[:args].map(&method(:to_decl)).join(', ') + ') {'

  out.puts '  ' + log_args(args)

  if is_variadic(function)
    out.puts "  va_list _args_;"
    out.puts "  va_start(_args_, #{args[-2][:name]});"
  end

  if function[:type] != 'void'
    out.puts "  #{to_shim_type(function[:type])} _ret_ = "
  end

  if shim_fun_impl
    out.print "  #{shim_fun_impl}"
  else
    out.print '  '
    if is_variadic(function)
      out.print 'v'
    end
    out.print function[:name]
  end
  out.print '('
  out.print (args.map do |arg|
    if arg[:name] == '...'
      '_args_'
    else
      arg[:name]
    end
  end).join(', ')
  out.puts ');'

  if is_variadic(function)
    out.puts '  va_end(_args_);'
  end

  out.puts '  ' + log_result(function)

  if function[:type] != 'void'
    out.puts '  return _ret_;'
  end

  out.puts '}'
end
