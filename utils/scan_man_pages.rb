#!/usr/bin/env ruby
# encoding: utf-8

require 'json'
require 'open3'
require(__dir__ + '/_prototypes.rb')

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
