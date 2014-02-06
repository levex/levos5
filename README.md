levos5
======

LevOS5 - A UNIX-like kernel aiming for POSIX compliance.

Compilation
===========

You need:
* i586-elf GCC and binutils for x86 build.
* arm-elf GCC and binutils for ARM build.

Run 'make' to build the kernel for the x86 (default) architecture.

You can specify the architecture with 'make ARCH={arm,x86}'

Features
========

* Portability support
* TTY layer
* PS/2 keyboard driver
* Interrupt processing
* x86 support
* ARM support
* Liballoc
* Display abstraction
* Multitasking
* Floppy support
* Virtual filesystem
* EXT2 filesystem support
* Simple /proc with pseudo-files
* ELF executable loading
* fork() system call
* initrd


Portability support
===================

This layer allows LevOS5 to not actually depend on the architecture for various things. This is done by the makefile defining a $ARCH variable, on which the makefile will decide which object to link in. The 'include/hal.h' file has the methods that must be implemented by the specific $ARCH.

Each $ARCH has two hooks into the system initialization. arch_early_init() and arch_late_init().
arch_early_init() has to set the system in a state that memory can be used and the tty layer can be fired up. No output is available during this stage.
arch_late_init() has to set the system to a state where tasking can be started with the $ARCH specific timer.

TTY layer
=========

Buffered tty. TTYs are the top layer of LevOS5. Each tty can be connected to one display and only one tty can be active (accepting input, flushing output to underlying display) at a time. 

PS/2 keyboard driver
====================

A simple buffered keyboard driver. Handles the IRQ and pushes the ASCII code to a FIFO buffer. Supporting uppercase with
shift, and CAPSLOCK as well. Current scancode set is the ENGLISH keyboard.

Interrupt processing
====================

Each $ARCH implements its own interrupt_ctl(int enable) method which allows for interrupt(or the alike) to be switched on and off. For example, on x86 this is done by the sti and cli instructions.

Each $ARCH must also implement its own register_interrupt(int intno, void(*f)()) which will register an interrupt handler. On X86, this is done by adding the field to the IDT.

x86 support
===========

This is the original architecture of LevOS5. Currently supports the GDT, IDT, and paging.
mm_{alloc, free}_pages are implemented with paging and bitmap handling.

ARM support
===========

This is a simple test that shows how easily LevOS5 can be ported. :-)

Check out arch/arm/* for details.

Liballoc
========

LevOS5 uses the public domain code of LibAlloc for malloc, realloc, calloc and free. These function are very accurate and extremely fast.

Display abstraction
===================

LevOS5 uses alot of layers for abstraction. First there is the display layer which will copy from the tty buffer and display the data stored in that tty buffer. The actual output is based on how the display handles the buffer of the tty. This way LevOS5 doesn't need to care about the actual display stuff, but rather write to the tty buffer and flush that if the display needs to be updated.

Multitasking
============

LevOS5 uses preemptive multitasking in kernel mode. This is currently only supported on the x86 architecture, but will be soon implemented on the ARM architecture as well. Scheduling is done by making the architecture call 'scheduler_switch()' and then let the rest do the magic. :-) LevOS5 uses a simple round robin algorithm for scheduling and supports a maximum of 16 processes each with 16 threads. However, currently each process can contain one thread.

Floppy support
==============

LevOS can read (and write) to floppy disks. Currently only one floppy disk is supported. This driver is temporarily in
use, we shall switch to ATA once I finish that driver. The floppy support also includes a DMA driver, which is
Direct-Memory-Access.

Virtual filesystem
==================

The virtual filesystem allows the mounting of 32 devices. Devices can beof different filesystems. The passed path to
device_try_to_mount() will need to NOT exist. This allows you to mount so called floaty-mounts.


Ext2 filesystem support
=======================

LevOS5.0 can mount and read ext2 filesystems. Directories are handled with care, but they are stable. Unexisting files
passed might cause a few errors and crashes, but the updated exception handler should prevent the computer from
crashing.

Procfs
======

Inside /proc, you will find a devconf file, to which you can write usingopen(2) and write(2). You will be able to
read(2) the data back once you finish writing.

Elf executable loading
======================

ELF executables are properly parsed and loaded to memory. Since LevOS implements execve(2) you can pass parameters to
applications. However, environment variables are not yet supported.

fork()
=====

Fork is a system call to allow a process to 'fork', i.e. create a new process which starts execution from where the
parent is. This is a very powerful system call, and has been implemented in LevOS.

initrd
======

LevOS can boot from an initrd, that will be mounted by the ext2 driver. No other formatting is supported at the moment.
:(


