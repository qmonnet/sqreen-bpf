# Virtual Machine for the Workshop

For any attendee who may experience issues when setting up the working
environment for the workshop, or who prefers to skip the installation bits, a
virtual machine is available.

## Contents

This is an image of a Ubuntu 18.04 server machine, with all tools necessary for
the workshop (bpftool, bcc, bpftrace) installed.

## Get the VM Image

The image can be retrieved [from this link](https://qoba.lt/dl/sqreen.qcow2)
(5.9 GB) (sha1sum: 5d9a870dcacfe915d2bab98d4c55b1497af84a93).

If you prefer to install the tools yourself, an alternative, lighter version is
also available [here](https://qoba.lt/dl/sqreen_bare.qcow2) (3.6 GB) (sha1sum:
f31b594f1e683acc0f88f85a50b9cc90cdc86f97).

Note that both will be removed from the server after the workshop is over.

## Launch the Image

This is a Qemu image. You can launch it with the following command:

    $ qemu-system-x86_64 -m 1024 -enable-kvm -net nic -net user,hostfwd=tcp::1605-:22 -hda sqreen.qcow2

The `hostfwd` option redirects the host TCP port 1605 to the guest port 22,
which means that once the machine boots, you can SSH into it from your machine:

    $ ssh root@localhost -p 1605

There are two accounts: `sqreen` and `root`. Both have `sqreen` as password.
Because all BPF tracing commands require admin privileges, it is recommended to
login and work as `root` for the workshop.
