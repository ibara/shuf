shuf
====
`shuf` is a utility that outputs a random permutation of its input lines.

It is a BSD-licensed implementation of the `shuf(1)` utility from GNU
coreutils.

Usage
-----
```
shuf [-hv] [-n count] [-o outfile] [-rz] [file]
shuf [-hv] -e [-n count] [-o outfile] [-rz] [args ...]
shuf [-hv] -i lo-hi [-n count] [-o outfile] [-rz]
```
See `shuf.1` for more details.

Dependencies
------------
Because of its use of OpenBSD
[arc4random_uniform(3)](https://man.openbsd.org/arc4random_uniform.3),
[getprogname(3)](https://man.openbsd.org/getprogname.3), and
[strtonum(3)](https://man.openbsd.org/strtonum.3),
non-\*BSD users may need to install
[libbsd](https://libbsd.freedesktop.org/) to compile `shuf`.

`configure` will figure this out for you.

Compiling
---------
```
./configure
$ make
$ sudo make install
```

Reporting bugs
--------------
Patches always welcome.

`shuf` should not crash on any input. `shuf` has been extensively fuzzed with
[`afl-fuzz`](http://lcamtuf.coredump.cx/afl/) but more testing is always
better. Please file a bug report with crashing input if you find any.

`shuf` should perfectly mimic `GNU shuf` behavior, with minimal exceptions
(notably, `shuf` provides better error messages). If you discover any areas
where `shuf` does not mimic `GNU shuf`, please file a bug report.

License
-------
ISC license. See `LICENSE` for details.

Download
--------
Tarballs are available here: https://devio.us/~bcallah/shuf/

Source code
-----------
Source code is hosted on GitHub: https://github.com/ibara/shuf
