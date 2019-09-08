# Discovering BPF Tracing with bcc

## Playing Around with bcc Tools

[bcc](https://github.com/iovisor/bcc) offers a set of wrappers to build
BPF-based tools, but it also comes with a set of pre-built tools and examples.
Let's try a few of them.

### Hello, World

Run the `hello_world` example.

    $ cd bcc
    # ./examples/hello_world.py

If nothing happens, open any file in another terminal.

Read the code for the tool and try to understand what happens.

### Sniffing SSL Because it's Fun

Launch the `sslsniff` tool, which relies on user probes. Check that it works,
e.g. with `curl https://wttr.in/paris`.

    # ./tools/sslsniff.py -n

### Snooping Around Opensnoop

Launch the `opensnoop` tool. Check that it works by opening files in another
terminal.

    # ./tools/opensnoop.py

While `opensnoop` is running, list BPF programs on the system.

    # bpftool prog show

`bpftool` has an option to print its output in JSON. Try the following.

    # bpftool prog -p

When `opensnoop` is running, we should see two `kprobe` programs loaded on the
system (one is actually a `kretprobe`, but `bpftool` does not make the
difference). We can dump the BPF bytecode loaded in the kernel.

    # bpftool prog dump xlated id <id>

If the JIT-compiler of the kernel was activated when we launched `opensnoop`
(if `# sysctl net.core.bpf_jit_enable` says `1`), then we can also dump the
JIT-ed instructions.

    # bpftool prog dump jited id <id>

### Counting IP Packets

Print the number of times function `inet_recvmsg()` has been called.

    # ./tools/funccount.py inet_recvmsg

(Pass a duration with `-d` or hit `Ctrl-C` to stop and print the result.)

Before stopping the program, list the BPF maps on the system.

    # bpftool map show

One of them should be an array called counts. We can dump it.

    # bpftool map dump id <id>

Guess what is the meaning of the value in the single entry?

### Tracing Memory Leaks

See the script `tests/python/test_tools_memleak.py`. Compile a program with
memory leaks and try to debug it with `memleak` and launch it.

    $ cd tests/python
    $ clang -g -O0 -o test_tools_memleak_leaker_app test_tools_memleak_leaker_app.c
    $ ./test_tools_memleak_leaker_app malloc

This sample application waits for the user to hit `Enter` before it runs, so
that we can attach `memleak` to it. Do so in a different terminal.

    $ ps aux | grep leaker_app
    $ cd bcc
    # ./tools/memleak.py -p <pid>

Hit `Enter` in the first terminal, where `test_tools_memleak_leaker_app` is
waiting.

(Note: in my latest tests I do not see the names of the function in the stack,
they are all at `[unknown]`, which does not help much for debug. I am not sure
why. I have seen function names before, and we can see some if we run `memleak`
without attaching it to a particular process anyway. I have not found the
explanation yet.)

### Tons of Programs!

Have a look at [the list](https://github.com/iovisor/bcc/#tools) of programs
packaged with bcc. Some programs print histograms. Some can be used to create
flame graphs. A lot of them anyway can be used to measure metrics that can turn
essential when trying to debug issues or to solve performance bottlenecks.

## Additional Resources

Two tutorials are available in the bcc repository.

[The first one](https://github.com/iovisor/bcc/blob/master/docs/tutorial.md)
explains how to get started with bcc tools in order to troubleshoot performance
issues, for example.

[The second
tutorial](https://github.com/iovisor/bcc/blob/master/docs/tutorial_bcc_python_developer.md)
is more oriented towards developers trying to build new tools with the Python
wrappers provided by the framework.

## Part 2

[bpftrace](exercises_bpftrace.md)
