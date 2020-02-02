#!/usr/bin/env ruby
# encoding: UTF-8

SHA256 = {
  'NVIDIA-Linux-x86_64-390.87.run': '38290a0b19e86037112f22397e02be9cdd6bcd677dd926b2977bbd49119e611e',
  'NVIDIA-Linux-x86_64-396.54.run': '7c3e3c9ca6aea31ad2613ef9cb6a4510d95d61b698f084e55adcc0611ee9eec3',
  'NVIDIA-Linux-x86_64-410.93.run': '33bee52be25dc680e9f83997a0faa00ebdaa800c51c7131428900809721e2161',
  'NVIDIA-Linux-x86_64-415.25.run': '64b65b8923d15e2566ed2ab6a4a65b21c45445bdd028ec0091c9b509a51e9349',
  'NVIDIA-Linux-x86_64-418.74.run': 'dc8736decb9627e18d71a5b3cb1273682d649208eca7dce0b589cd7faf20120f',
  'NVIDIA-Linux-x86_64-430.40.run': 'f700899f48ba711b7e1598014e8db9a93537d7baa3d6a64067ed08578387dfd7',
  'NVIDIA-Linux-x86_64-430.50.run': '0c683394ae47d145f989b74120b482431fa5c7730d41fb3433d8fd68324d3dc5',
  'NVIDIA-Linux-x86_64-435.17.run': 'a71cecb5b8f0af35ed9a2d4023652a0537271457ef570c5f21dccd5067d9e9a6',
  'NVIDIA-Linux-x86_64-440.26.run': '5f98e7af0643aea8c5538128bfef2f70dab0cd2559c97d0a8598210a3761c32b',
  'NVIDIA-Linux-x86_64-440.31.run': 'cd592f385c9bfb798f973225dfd086654973984762b669c7d765b074c0d8850f'
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

libs = []

if driver_version.split('.').first.to_i < 435
  libs << 'libGL.so.' + driver_version # legacy
end

libs += [
  'libGLX_nvidia.so.'    + driver_version, # glvnd
  'libnvidia-glcore.so.' + driver_version
]

if driver_version.split('.').first.to_i >= 396
  libs << 'libnvidia-glvkspirv.so.' + driver_version
end

if driver_version.split('.').first.to_i >= 415
  libs << 'libnvidia-tls.so.' + driver_version
else
  libs << 'tls/libnvidia-tls.so.' + driver_version
end

libs64 = libs.dup
libs32 = libs.map{|lib| '32/' + lib}

if driver_version.split('.').first.to_i >= 410
  libs64 << 'libnvidia-cbl.so.'             + driver_version
  libs64 << 'libnvidia-fatbinaryloader.so.' + driver_version
  libs64 << 'libnvidia-ptxjitcompiler.so.'  + driver_version
  libs64 << 'libnvidia-rtcore.so.'          + driver_version
end

run("tail -n +#{skip} #{installer} | xz -d | tar -C #{lib64_dir} -xf -                      #{libs64.join(' ')}")
run("tail -n +#{skip} #{installer} | xz -d | tar -C #{lib32_dir} -xf - --strip-components 2 #{libs32.join(' ')}")

puts 'Applying patches...'

# Apparently, libGL.so expects the .init section to be called with argc and argv arguments,
# FreeBSD's dynamic linker doesn't quite agree with that.
# https://sourceware.org/git/?p=glibc.git;a=blob;f=elf/dl-init.c;h=3e72fa3013a6aaeda05fe61a0ae7af5d46640826;hb=HEAD#l58
# https://github.com/freebsd/freebsd/blob/b0b07656b594066e09c0526aef09dc1e9703e27d/libexec/rtld-elf/rtld.c#L2655
# https://github.com/freebsd/freebsd/blob/35326d3159b53afb3e64a9926a953b32e27852c9/libexec/rtld-elf/amd64/rtld_machdep.h#L50

DT_INIT = 12
DT_FINI = 13

def patch_init(path)

  def to_number(slice)
    case slice.length
      when 8 then slice.unpack('Q<')[0]
      when 4 then slice.unpack('L<')[0]
      else
        raise
    end
  end

  def to_slice(number, wordsize)
    case wordsize
      when 8 then [number].pack('Q<')
      when 4 then [number].pack('L<')
      else
        raise
    end
  end

  def read_dynamic_section(obj, section_offset, wordsize)

    entries = []

    i = 0
    while true
      pos = section_offset + wordsize * 2 * i
      tag = to_number(obj[ pos            ...(pos + wordsize    )]);
      val = to_number(obj[(pos + wordsize)...(pos + wordsize * 2)]);

      entries << {offset: pos, tag: tag, val: val}

      break if tag == 0

      i += 1
    end

    entries
  end

  def wwrite(obj, offset, number, wordsize)
    pos = offset
    for char in to_slice(number, wordsize).chars
      obj[pos] = char
      pos += 1
    end
  end

  headers = `readelf --headers --wide "#{path}"`

  headers =~ /Class:\s+(ELF32|ELF64)/
  wordsize = $1 == 'ELF64' ? 8 : 4

  headers =~ /.dynamic\s+DYNAMIC\s+\w+\s(\w+)/
  section_offset = $1.to_i(16)

  obj = IO.binread(path)

  dynamic = read_dynamic_section(obj, section_offset, wordsize)

  init = dynamic.find{|e| e[:tag] == DT_INIT}
  fini = dynamic.find{|e| e[:tag] == DT_FINI}

  if init[:offset] < fini[:offset]
    wwrite(obj, init[:offset], DT_FINI, wordsize)
  else
    raise
  end

  IO.binwrite(path, obj)
end

# legacy
if driver_version.split('.').first.to_i < 435
  patch_init("#{lib64_dir}/libGL.so.#{driver_version}")
  patch_init("#{lib32_dir}/libGL.so.#{driver_version}")
end

# glvnd
patch_init("#{lib64_dir}/libGLX_nvidia.so.#{driver_version}")
patch_init("#{lib32_dir}/libGLX_nvidia.so.#{driver_version}")

puts 'Done'
