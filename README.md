This repository contains a shim library providing limited glibc ABI compatibility on top of FreeBSD libc.

The library is intended to allow loading shared objects compiled on Linux and dynamically linked to glibc
into native FreeBSD processes for a few use cases that warrant such a hack. Although FreeBSD already has
a very useful Linux compatibility feature, this kind of ad hoc ABI mixing is out of scope for it.

The implementation consists of const/struct conversions and stubs for functionality not existing in FreeBSD.
It's not particularly robust. Please, don't use this in a security-sensitive context.

For better stability make sure that your Linux objects:
- do not do direct syscalls (vs using libc syscall wrapper);
- do not poke opaque data structures;
- do not pass libc constants or structs to FreeBSD objects they are linked with.

Only i386/amd64 binaries are supported.

## Dependencies

FreeBSD, *ruby*. Additionally, `bin/nv-sglrun` requires *nvidia-driver* and *linux-nvidia-libs*.

## Usage

```
% make
% [env SHIM_DEBUG=1] ./bin/<with-glibc-shim | nv-sglrun> <application>
```
