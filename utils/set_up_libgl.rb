#!/usr/bin/env ruby
# encoding: UTF-8

SHA256 = {
  'NVIDIA-Linux-x86_64-390.77.run': 'e51c5784520e73c179a57cf0dcd3a4c673d8142b28415a060066d83633637282',
  'NVIDIA-Linux-x86_64-390.87.run': '38290a0b19e86037112f22397e02be9cdd6bcd677dd926b2977bbd49119e611e',
  'NVIDIA-Linux-x86_64-396.45.run': '30c2e29a45794227079730eac7c452da5290ea8f336ed2286dafd488e3695f20',
  'NVIDIA-Linux-x86_64-396.51.run': '6add8c3782bdf276b4a5a5bcea102ceff8e90cf3fc2439dde9d5e60d557ac0d6',
  'NVIDIA-Linux-x86_64-396.54.run': '7c3e3c9ca6aea31ad2613ef9cb6a4510d95d61b698f084e55adcc0611ee9eec3',
  'NVIDIA-Linux-x86_64-410.57.run': '5c3c2e1fef0615c0002946c586c815a77676f4683304cc17d5bf323e7626a320'
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
  'libGL.so.'            + driver_version,
  'libnvidia-glcore.so.' + driver_version,
  'libnvidia-tls.so.'    + driver_version
]

if driver_version.split('.').first.to_i >= 396
  libs64 << 'libnvidia-glvkspirv.so.' + driver_version
end

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

# Apparently, libGL.so expects the .init section to be called with argc and argv arguments,
# FreeBSD's dynamic linker doesn't quite agree with that.
# https://sourceware.org/git/?p=glibc.git;a=blob;f=elf/dl-init.c;h=3e72fa3013a6aaeda05fe61a0ae7af5d46640826;hb=HEAD#l58
# https://github.com/freebsd/freebsd/blob/b0b07656b594066e09c0526aef09dc1e9703e27d/libexec/rtld-elf/rtld.c#L2655
# https://github.com/freebsd/freebsd/blob/35326d3159b53afb3e64a9926a953b32e27852c9/libexec/rtld-elf/amd64/rtld_machdep.h#L50

with_file("#{lib64_dir}/libGL.so.#{driver_version}") do |lib|
  case driver_version
    when '390.77'
      lib[708608] = RET
    when '390.87'
      lib[708608] = RET
    when '396.45'
      lib[708240] = RET
    when '396.51'
      lib[708352] = RET
    when '396.54'
      lib[708352] = RET
    when '410.57'
      lib[708720] = RET
    else
      raise
  end
end

with_file("#{lib32_dir}/libGL.so.#{driver_version}") do |lib|
  case driver_version
    when '390.77'
      lib[689440] = RET
    when '390.87'
      lib[689440] = RET
    when '396.45'
      lib[690576] = RET
    when '396.51'
      lib[690576] = RET
    when '396.54'
      lib[690576] = RET
    when '410.57'
      lib[689904] = RET
    else
      raise
  end
end

puts 'Done'
