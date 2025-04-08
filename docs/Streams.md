# Handling file streams

This document basically outlines the various different file reading techniques that are present and how can we write an
efficient and safe code for the same.

Each section deals with one particular way of doing this

# Loading entire file into memory

In this method the main idea is that given the location of file - just load it into memory and then perform all your
operations as all it takes is one single Filesystem call - which would bring everything to memory and then fast access
to process all the data.

However the drawback for the same is that we do not control the file size leading to an outage of memory for extremely
large files.

> Final Verdict: NO

# Chunk based processing

In this case as we can no longer bring the entire file to memory we would have manually decide a chunk size on our
application and then read a file to load that chunk at a time - this possible via system calls and then different sizes
in bytes that is provided to system call which would then ask the hardware to pull the data from disk and load it to RAM
.

While this efficient in theory a lot of manual memory management is required and we have to do multiple system calls to
access the disk and load the data for the same. This is an ideal case scenario but with manual memory management we have
to ensure that all the memory read has to be deallocated as well which requires specific programming techniques or 
already available abstractions.

> Final Verdict: Good enough

# Memory mapped File Processing

This support is directly present with the kernel - where we can tell the kernel what file we would like to read and just
ask it to provide us support for the entire file as if it is on memory. This gives us a support for entire file loaded
onto memory and we can focus on actually reading the file and performing the operation.

However it actually does not load the entire file at the same time rather - the processing is being done in chunks where
the kernel actually creates entries in its page table of all the different content that needs to be loaded based on the
page size of the system.

As it just creates an entries in the table at the beginning and does not actually load the data that means we have to
try to access the data and at that point there will be a page fault and then the Kernel is responsible for actually
loading the pages onto the memory after which we would perform our operation.

> Final Verdict: Good enough

# Final thoughts

As we are trying to improve our skills by completing out this challenge we should implement all three ways and then via
class design and what needs to be compiled/provided as option can be taken into consideration and we can then measure
performance for each of them and provide a compiled report for the same.
