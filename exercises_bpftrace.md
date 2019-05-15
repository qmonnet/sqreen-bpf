# Enjoying One-Liners with bpftrace

While bcc still requires writing BPF programs,
[bpftrace](https://github.com/iovisor/bpftrace) is a higher-level tool that
reuses features offered by bcc in order to provide a simple command-line tool
for tracing with BPF.

The
[reference guide](https://github.com/iovisor/bpftrace/blob/master/docs/reference_guide.md)
for bpftrace provides exhaustive documentation about the syntax of the tool and
the built-in functions and variables.

## All the Probes in the World

List all probes supported by bpftrace.

    # bpftrace -l | less

Note the software perf\_events, the hardware counters, the tracepoints, the
kprobes. Obviously bpftrace is not aware of all possible user probes, and does
not list any.

List the probes related to BPF tracepoints in the kernel.

    # bpftrace -l "tracepoint:bpf*"

## Opensnoop, bpftrace Edition

Launch the bpftrace version of `opensnoop`.

    # bpftrace -e 'kprobe:do_sys_open { printf("%d - %s: %s\n", pid, comm, str(arg1)) }'

## Snoop in the Shadows

Edit the previous command to print only when `open()` is used on file
`/etc/shadow`.

## Tracepoint:syscalls:sys\_enter\_execve

Trace all processes executed by a non-root user.

## Additional Resources

Beside the reference guide, there is
[a tutorial for one-liners](https://github.com/iovisor/bpftrace/blob/master/docs/tutorial_one_lines.md)
with bpftrace, that provides additional commands to try. Totally worth a read.
Let's draw histograms!

## Part 3

[Network processing](exercises_net.md)
