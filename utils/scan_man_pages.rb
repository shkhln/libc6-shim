#!/usr/bin/env ruby
# encoding: utf-8

require 'json'
require 'open3'

PAGE_BLACKLIST = [
  'KQUEUE(2)',
  'NTP_ADJTIME(2)',
  'RPC_SVC_REG(3)',
  'SIGACTION(2)',
  'SIGPAUSE(2)',
  'SIGVEC(2)',
  'SIGNAL(3)'
]

PAGE_WHITELIST = [
  'JEMALLOC(3)',
  'DIRNAME(3)' # no Library section
]

PROTOTYPE_BLACKLIST = [
  /FD_SET\(fd, &fdset\)/,
  /FD_CLR\(fd, &fdset\)/,
  /FD_ISSET\(fd, &fdset\)/,
  /FD_ZERO\(&fdset\)/,
  /Standard API/,
  /Non-standard API/,
  /const char *malloc_conf/,
  /real\-floating/,
  /easier to read typedef\'d version/,
  /int exec(l|lp|le)\(/, # comments
  /scandir_b/, # unbalanced parens
  /void \(\*malloc_message\)\(void \*cbopaque, const char \*s\)/
]

def is_function_prototype(str)
  str.include?('(') && !str.start_with?('typedef') && !str.start_with?('#')
end

def split_prototype(prototype)

  parts = []

  depth = 0
  decl  = ''

  prototype.chars.each_with_index do |char, i|
    case char
      when '('
        if depth == 0
          parts << decl
          decl = ''
        else
          decl += '('
        end
        depth += 1
      when ')'
        depth -= 1
        if depth == 0
          parts << decl
        else
          decl += ')'
        end
      when ','
        if depth == 1
          parts << decl
          decl = ''
        else
          decl += ','
        end
      when /[\w\s\*\^\[\]\.\+]/i
        decl += char
      else
        if char.ord == 0xa0
          decl += ' '
        else
          raise "Unexpected character '#{char}' (0x#{'%x' % char.ord}) at pos #{i + 1} in #{prototype.inspect}"
        end
    end
  end

  parts.map(&:strip)
end

FUNCTION_POINTER_DECL = /^(.+)?\([\*\^](\w+)\)\s*(\([^\)]+\))$/

def parse_synopsis(synopsis)

  includes  = synopsis.split(/\n/)       .map(&:strip).map{|e| e.gsub(/\s+/, ' ')}.find_all{|entry| entry =~ /#include/}
  functions = synopsis.split(/(\n\n+|;)/).map(&:strip).map{|e| e.gsub(/\s+/, ' ')}.find_all(&method(:is_function_prototype))

  functions = functions.find_all{|str| !PROTOTYPE_BLACKLIST.any?{|pattern| str =~ pattern}}

  positions = {}
  for str in includes + functions
    positions[str] = synopsis.gsub(/\s+/, ' ').index(str)
  end

  functions.map do |prototype|

    fn_includes = includes.find_all{|include| positions[include] < positions[prototype]}

    fn_type_name, *args = split_prototype(prototype).select{|p| !p.empty?}.to_enum.with_index.map do |decl, i|
      begin
        name = nil
        type = nil

        case decl
          when /\*$/, /\*\srestrict$/
            name = "_arg_#{i}"
            type = decl
          when 'void'
            type = 'void'
          when '...'
            name = '...'
          when /^[^\s]+$/
            name = "_arg_#{i}"
            type = decl
          when FUNCTION_POINTER_DECL
            name = $2
            type = $1 + '(*)' + $3
          when /^(.+?)(\w+)$/
            name = $2
            type = $1.strip
          when /^(.+?)(\w+)(\[(\d*|restrict)\])$/
            name = $2
            type = $1 + $3
          else
            raise "Unknown decl #{decl.inspect}"
        end

        {name: name, type: type}
      rescue
        STDERR.puts "in \"#{prototype}\""
        raise
      end
    end

    {prototype: prototype, name: fn_type_name[:name],type: fn_type_name[:type], args: args, includes: fn_includes.reverse.uniq.reverse}
  end
end

visited = {}
json    = {}

man_pages = Dir['/usr/share/man/en.UTF-8/man2/*.2.gz'] + Dir['/usr/share/man/en.UTF-8/man3/*.3.gz']
man_pages.sort!
man_pages.uniq!

for page in man_pages

  md5 = `md5 -q #{page}`

  next if visited[md5]
  visited[md5] = true

  out, _, _ = Open3.capture3("zcat #{page} | mandoc -T utf8")

  out = out
    .force_encoding('UTF-8')
    .gsub("\xC2\xA0", ' ')
    .gsub(/.[\b]/, '')

  manual = nil
  manual = [$1, $2.strip] if out.lines.first =~ /^(\w+\(\d\))(.+?)(\w+\(\d\))$/i
  manual = [$1, $2.strip] if out.lines.first =~ /^(\w+\(\d\))(.+?)$/i

  sections = {}
  section  = nil

  for line in out.lines[1..-1]
    if line =~ /^([A-Z]+)/
      section = $1
    else
      sections[section] = '' if not sections[section]
      sections[section] += line
    end
  end

  next if not manual
  next if PAGE_BLACKLIST.include?(manual[0])

  if PAGE_WHITELIST.include?(manual[0]) || (manual[1] =~ /FreeBSD (System Calls|Library Functions)/ && sections['LIBRARY'] =~ /(Standard C|Math|POSIX Threads)/)
    begin
      json[manual[0]] = {
        functions: parse_synopsis(sections['SYNOPSIS'] || '')
      }
    rescue
      STDERR.puts "in #{manual[0]}"
      raise
    end
  end
end

puts JSON.pretty_generate(json)
