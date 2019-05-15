# All-Out Programmability in Linux with BPF

A presentation about BPF, with a focus on the Linux tracing aspects, made at
[Sqreen](https://www.sqreen.com/) offices in May 2019.

## Slides

[Slides for the presentation](sqreen-slides.pdf)

## Workshop

### Set Up

This workshop was prepared on a machine running Ubuntu 18.04 server edition
(kernel 4.15). It should work on other distributions as well. A recent kernel
is required (4.15 works, a few versions before that may work, but it is
recommended to use the newest kernel available).

If you do not have access to a suitable machine, a virtual image is made
available for duration of the workshop, see [vm.md](vm.md).

### Installing the Tools

This workshop revolves around three tools:

* bpftool
* bcc
* bpftrace

The first step consists in installing them. Please refer to
[quick\_install.md](quick_install.md) for instructions.

The provided VM already has all three tools installed.

### Exercises

Note that most of the work will require admin privileges. If in a suitable
environment, you may want to work as root.

* [First part: bcc](exercises_bcc.md)

* [Second part: bpftrace](exercises_bpftrace.md)

* [Third part: networking](exercises_net.md)

* Finished early? Just play around with the tools! Try creating a
  [flame graph](https://github.com/brendangregg/FlameGraph) with `profile` or
  `offcputime` from bcc for example?
