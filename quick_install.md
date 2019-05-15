Installing BPF Tools
====================

These instructions were tested on a fresh Ubuntu 18.04 server install. The
exact set of dependencies for other distributions may vary. Please refer to the
official documentation of the software for more information.

LLVM
----

bpftrace has [an issue](https://github.com/iovisor/bpftrace/issues/76) with
LLVM 6.0, which is the newest available version in Ubuntu repositories. Let's
install version 8 instead.

See also [LLVM APT repository](http://apt.llvm.org/).

    wget -O - https://apt.llvm.org/llvm-snapshot.gpg.key|sudo apt-key add -
    sed -i '$a deb http://apt.llvm.org/bionic/ llvm-toolchain-bionic-8 main' /etc/apt/sources.list
    apt update

Additionally, we can alias `clang` to `clang-8` (not required):

    LLVM_VERSION=8
    update-alternatives --install /usr/bin/clang clang "/usr/bin/clang-$LLVM_VERSION" 100
    update-alternatives --install /usr/bin/clang++ clang++ "/usr/bin/clang++-$LLVM_VERSION" 100
    update-alternatives --install /usr/bin/llc llc "/usr/bin/llc-$LLVM_VERSION" 100
    update-alternatives --install /usr/bin/llvm-mc llvm-mc "/usr/bin/llvm-mc-$LLVM_VERSION" 50

bcc
---

See also [bcc documentation](https://github.com/iovisor/bcc/blob/master/INSTALL.md).

Packages are available for some distributions. The Ubuntu packages do not seem
to work consistently, I would recommend building bcc from the sources.

    apt install python bison build-essential cmake flex libedit-dev libllvm8 llvm-8-dev libclang-8-dev zlib1g-dev libelf-dev
    git clone https://github.com/iovisor/bcc.git
    mkdir bcc/build
    cd bcc/build
    cmake .. -DCMAKE_INSTALL_PREFIX=/usr
    make
    make install

bpftrace
--------

See also [bpftrace documentation](https://github.com/iovisor/bpftrace/blob/master/INSTALL.md).

Packages are available for several distributions.

bpftrace requires bcc to be installed beforehand.

    apt install libfl-dev systemtap-sdt-dev clang-8
    git clone https://github.com/iovisor/bpftrace.git
    mkdir bpftrace/build
    cd bpftrace/build
    cmake .. -DCMAKE_BUILD_TYPE=Release
    make
    make install

bpftool
-------

### Packages

Fedora has a package for bpftool.

For Debian/Ubuntu: There is a .deb package available from Netronome website.
The binary is statically compiled and should work on all distributions.

    wget https://help.netronome.com/helpdesk/attachments/36025601060 -O bpftool-4.20_amd64.deb
    dpkg -i bpftool-4.20_amd64.deb

### From Ubuntu sources

Compiling bpftool requires downloading sources for the Linux kernel.
Also, it seems that the version shipped with kernel 4.15 does not build
properly, and must be patched.

    apt install binutils-dev libelf-dev
    apt source linux
    cd linux-4.15.0/tools/bpf/bpftool

    patch -p1 jit_disasm.c << EOF
    --- jit_disasm.c
    +++ jit_disasm.c
    @@ -107,7 +107,10 @@
     
     	disassemble_init_for_target(&info);
     
    -	disassemble = disassembler(bfdf);
    +	disassemble = disassembler(info.arch,
    +				   bfd_big_endian(bfdf),
    +				   info.mach,
    +				   bfdf);
     	assert(disassemble);
     
     	if (json_output)
    EOF

    make
    make doc
    make install doc-install

### From latest Linux sources

With the git kernel repository (Caution: takes a long time to check out!):

    apt install binutils-dev libelf-dev
    git://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git
    cd linux/tools/bpf/bpftool
    make
    make doc
    make install doc-install
