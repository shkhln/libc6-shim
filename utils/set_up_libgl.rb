#!/usr/bin/env ruby
# encoding: UTF-8

SHA256 = {
  'NVIDIA-Linux-x86_64-396.45.run': '30c2e29a45794227079730eac7c452da5290ea8f336ed2286dafd488e3695f20'
}

fetch_dir = __dir__ + '/../nvidia'
lib64_dir = __dir__ + '/../nvidia/lib64'
lib32_dir = __dir__ + '/../nvidia/lib32'

`sysctl hw.nvidia.version` =~ /^hw.nvidia.version: NVIDIA UNIX x86_64 Kernel Module  ([\d\.+]+)/
driver_version = $1

installer = "#{fetch_dir}/NVIDIA-Linux-x86_64-#{driver_version}.run"

def run(cmd)
  #~ sync = STDOUT.sync
  #~ STDOUT.sync = true
  IO.popen(cmd) do |pipe|
    pipe.sync = true
    while s = pipe.gets
      puts s
    end
  end
  #~ STDOUT.sync = sync
end

if not File.exists?(installer)

  dist_dir = File.dirname(installer)

  if not File.exists?(dist_dir)
    `mkdir -p #{dist_dir}`
  end

  puts "Downloading NVIDIA-Linux-x86_64-#{driver_version}.run..."
  pwd = Dir.pwd
  Dir.chdir(dist_dir)
  run("fetch https://download.nvidia.com/XFree86/Linux-x86_64/#{driver_version}/NVIDIA-Linux-x86_64-#{driver_version}.run")
  Dir.chdir(pwd)
end

print 'Verifying checksum...'

if `sha256 -q #{installer}`.strip == SHA256[File.basename(installer).to_sym]
  puts ' ok'
else
  puts ' nope'
  exit(1)
end

puts 'Extracting files...'

skip = IO.read(installer).lines[0..17].find{|line| line =~ /^skip=\d+$/}.strip.split('=').last

`mkdir #{lib64_dir}` if not File.exists?(lib64_dir)
`mkdir #{lib32_dir}` if not File.exists?(lib32_dir)

libs64 = [
  'libGL.so.'               + driver_version,
  'libnvidia-glcore.so.'    + driver_version,
  'libnvidia-glvkspirv.so.' + driver_version,
  'libnvidia-tls.so.'       + driver_version
]

libs32 = libs64.map{|lib| '32/' + lib}

run("tail -n +#{skip} #{installer} | xz -d | tar -C #{lib64_dir} -xf -                      #{libs64.join(' ')}")
run("tail -n +#{skip} #{installer} | xz -d | tar -C #{lib32_dir} -xf - --strip-components 2 #{libs32.join(' ')}")

puts 'Applying patches...'

def bin(str)
  str.force_encoding('BINARY')
end

RET = bin("\xC3")

def with_file(path)
  file = IO.binread(path)
  yield file
  IO.binwrite(path, file)
end

with_file("#{lib64_dir}/libGL.so.#{driver_version}") do |lib|
  case driver_version
    when '396.45'
      lib[708240] = RET
    else
      raise
  end
end

with_file("#{lib32_dir}/libGL.so.#{driver_version}") do |lib|
  case driver_version
    when '396.45'
      lib[690576] = RET
    else
      raise
  end
end

puts 'Done'
