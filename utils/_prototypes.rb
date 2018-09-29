#!/usr/bin/env ruby
# encoding: utf-8

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
