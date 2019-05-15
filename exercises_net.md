# Network Processing with BPF

## Sample TC Filter

### About TC binary

Note that this exercise requires a recent version of iproute2. The version from
Ubuntu 18.04 (iproute2-ss180129) does not seem to load maps correctly. You might want to compile a new version.

    # apt install pkg-config libmbl-dev libelf-dev
    $ git clone https://git.kernel.org/pub/scm/network/iproute2/iproute2.git
    $ cd iproute2
    $ ./configure
    $ make
    # make install

### Filtering

TC is the framework for Traffic Control on Linux. Let's create a program and
attach it to the interface `eth0`.

There is a program in file [port\_block.c](port_block.c). It parses IPv4 and
TCP/UDP headers of a packet (if present), and blocks those using a given port
number. This port number is passed via a map, and can be changed without even
reloading the program. A second map is used to store statistics.

First, we need to compile the program with clang.

    $ clang-8 -O2 -g -emit-llvm -c port_block.c -o - | \
            llc-8 -march=bpf -mcpu=probe -filetype=obj -o port_block.o

This should produce an ELF file `port_block.o`. We can inspect the BPF
bytecode stored in it with `llvm-objdump-8 -S port_block.o`.

TC works with queue disciplines (`qdisc`) that defines the algorithms used to
process the different packet queues, and with classifiers (filters) to direct
packets into the relevant queues.

Let's add a `qdisc` to our interface. For BPF, we use `cls_act`.

    # tc qdisc add dev eth0 clsact

Now let's add the BPF program stored in our object file as a filter for this
`qdisc`. Because `cls_act` is a bit special, we are not just classifying
packets, we are also applying an action (such as “drop” or “pass”) to each
packet.

    # tc filter add dev eth0 ingress bpf da obj port_block.o sec .text

The last step consists in adding a value to the map. We can do this with
`bpftool`.

    # bpftool map update id <block_port_map_id> key 0 0 0 0 value 80 0 0 0

Let's test the application by sending some traffic towards port 80. It should
be blocked. We can also look at the statistics from the other map.

    # bpftool map update id <stats_map_id> key 0 0 0 0

## Improving the Filter

This program was simple, but BPF is a powerful tool for network processing. Our
example could be changed to enforce rules on other fields. We could also have a
more complex table for the access rules: usually, we parse the source and
destination addresses and ports, and use the 5-tuple obtained by adding the L4
protocol number as a key to retrieve the relevant access rule from a hash
table.

It is also possible to implement operations like encapsulation and
decapsulation of the packets (there are BPF helpers to help with this task, in
particular to change the size of the packet). It is also possible to implement
a hashing function and to apply it onto the packet, for example to redirect
(load-balance) the packet towards a specific server.

Feel free to experiment.

## More Networking

There are a couple of XDP-related sample applications on
[Netronome's GitHub repository](https://github.com/Netronome/bpf-samples.git ).
The concept XDP is similar, except that the hook for the BPF programs is at the
driver level, enabling much better performance. The sample applications are
more complex than our very simple example, but they are designed as example for
understanding how to use XDP and should not be too difficult to understand.

Another place to search for tutorials is
[the XDP tutorial](https://github.com/xdp-project/xdp-tutorial.git) repository
on GitHub. It contains several “lessons“, ordered by level of difficulty.
