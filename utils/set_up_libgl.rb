#!/usr/bin/env ruby
# encoding: UTF-8

SHA256 = {
  'NVIDIA-Linux-x86_64-390.77.run': 'e51c5784520e73c179a57cf0dcd3a4c673d8142b28415a060066d83633637282',
  'NVIDIA-Linux-x86_64-390.87.run': '38290a0b19e86037112f22397e02be9cdd6bcd677dd926b2977bbd49119e611e',
  'NVIDIA-Linux-x86_64-396.45.run': '30c2e29a45794227079730eac7c452da5290ea8f336ed2286dafd488e3695f20',
  'NVIDIA-Linux-x86_64-396.51.run': '6add8c3782bdf276b4a5a5bcea102ceff8e90cf3fc2439dde9d5e60d557ac0d6',
  'NVIDIA-Linux-x86_64-396.54.run': '7c3e3c9ca6aea31ad2613ef9cb6a4510d95d61b698f084e55adcc0611ee9eec3',
  'NVIDIA-Linux-x86_64-410.57.run': '5c3c2e1fef0615c0002946c586c815a77676f4683304cc17d5bf323e7626a320',
  'NVIDIA-Linux-x86_64-410.66.run': '8fb6ad857fa9a93307adf3f44f5decddd0bf8587a7ad66c6bfb33e07e4feb217',
  'NVIDIA-Linux-x86_64-410.73.run': 'bebc9cf781201beb5ec1a1dde7672db68609b8af0aa5ff32daa3ebb533c2ff1e',
  'NVIDIA-Linux-x86_64-410.93.run': '33bee52be25dc680e9f83997a0faa00ebdaa800c51c7131428900809721e2161',
  'NVIDIA-Linux-x86_64-415.13.run': '2ad26d77b848e12a72b6fece320cb867a07a36096bd2e21f4a0c8fa40a51645a',
  'NVIDIA-Linux-x86_64-415.22.run': '22188f7e095b648554738638258107b89288b7902bdc3b4edaabbda615745b9b',
  'NVIDIA-Linux-x86_64-415.25.run': '64b65b8923d15e2566ed2ab6a4a65b21c45445bdd028ec0091c9b509a51e9349',
  'NVIDIA-Linux-x86_64-418.30.run': 'c1ade253080a049ea38149b9ceb8f20549b5b17164eeae114d6c058e01e23958',
  'NVIDIA-Linux-x86_64-418.43.run': '18be2c83dee3323bd57fe77fddbbbbd5d760ada674781fb9b39321e6386a327f',
  'NVIDIA-Linux-x86_64-418.56.run': 'c45500db69ff66b55612904d3c5d2beabf52f29799367f1fcf81d6f28e48e7b1',
  'NVIDIA-Linux-x86_64-418.74.run': 'dc8736decb9627e18d71a5b3cb1273682d649208eca7dce0b589cd7faf20120f',
  'NVIDIA-Linux-x86_64-430.09.run': '2565e5f0b0da5f16f1675f67bb05e2fa397d581d8ed9acb23248282f2954a94c',
  'NVIDIA-Linux-x86_64-430.14.run': '00d46ffaf3e1e430081ddbd68b74cc361cd1328e8944224dfe69630dd8540f17',
  'NVIDIA-Linux-x86_64-430.26.run': '66ab94a94436732e6e9e8a95a0f5418759a4c2abd2ede65a9472a8de08edcee6'
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
  'libGL.so.'             + driver_version,
  'libnvidia-glcore.so.'  + driver_version,
]

if driver_version.split('.').first.to_i >= 396
  libs64 << 'libnvidia-glvkspirv.so.' + driver_version
end

if driver_version.split('.').first.to_i >= 415
  libs64 << 'libnvidia-tls.so.' + driver_version
else
  libs64 << 'tls/libnvidia-tls.so.' + driver_version
end

libs32 = libs64.map{|lib| '32/' + lib}

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

patch_init("#{lib64_dir}/libGL.so.#{driver_version}")
patch_init("#{lib32_dir}/libGL.so.#{driver_version}")

for path in [
  "#{lib64_dir}/libnvidia-cbl.so.#{driver_version}",
  "#{lib64_dir}/libnvidia-glvkspirv.so.#{driver_version}",
]
  IO.binwrite(path, IO.binread(path)
    .gsub(Regexp.new("librt.so.1\0".force_encoding('BINARY'), Regexp::FIXEDENCODING), "librt.so.x\0"))
end

puts 'Done'
