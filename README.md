# COMP2002 - Parallel Merge Sort

**Authors:** Ayush Chauhan; Nhat Khang Nguyen (a1902467); Wentao

## Overview

This project implements merge sort in C using POSIX threads. The array is
recursively divided among threads until a user-selected cutoff level is
reached. Each remaining section is then sorted with single-threaded merge
sort, and the sorted sections are merged back together.

## Manifest

| File | Description |
| --- | --- |
| `mergesort.c` | Implements `merge`, recursive `my_mergesort`, threaded `parallel_mergesort`, and the `buildArgs` helper. |
| `mergesort.h` | Provides declarations, shared data structures, and global variable declarations. |
| `test-mergesort.c` | Generates input, invokes the sort, checks correctness, and reports execution time. |
| `Makefile` | Provides build rules for compiling `test-mergesort`. |
| `README.md` | Documents the project, build and usage instructions, testing, reflection, and sources. |
| `README.template` | Provides the supplied README structure and writing guidance. |
| `tests.txt` | Lists test runs for a range of input sizes, cutoff levels, and seeds. |

## Building the project

From the project directory, run:

```sh
make
```

This builds the `test-mergesort` executable. To remove generated files and the
executable, run:

```sh
make clean
```

## Features and usage

Run the program with:

```sh
./test-mergesort <input-size> <cutoff-level> <seed>
```

For example:

```sh
./test-mergesort 100000000 3 1234
```

The arguments specify the number of integers to sort, the thread cutoff level,
and the random-number seed. A cutoff of `0` gives the serial baseline; larger
values allow more levels of worker threads. The seed makes the generated input
repeatable across runs.

`parallel_mergesort` creates two child threads for the left and right
subarrays while the current level is below the cutoff. At the cutoff, it
switches to `my_mergesort`. Each parent waits for both children with
`pthread_join` before merging their sorted ranges.

## Testing

Correctness was tested using the supplied `test-mergesort` program with
multiple input sizes, cutoff levels, and seeds. The tester's
`check_if_sorted` function verifies that the final array is in nondecreasing
order.

Small and medium inputs were used to check serial and parallel execution at
cutoff levels 0, 1, 2, and 3. Larger tests used 100,000,000 elements as
required by the specification.

Example performance results on a MacBook Pro using seed `1234` were:

| Cutoff | Time (100,000,000 elements) | Approx. speedup vs. cutoff 0 |
| ---: | ---: | ---: |
| 0 | 16.92 s | 1.00x |
| 1 | 8.79 s | 1.93x |
| 2 | 4.89 s | 3.46x |
| 3 | 3.07 s | 5.51x |
| 4 | 3.41 s | 4.96x |
| 5 | 3.11 s | 5.44x |

These results show that increasing the cutoff does not always improve
performance. Once enough parallel work is available to occupy the processor,
additional threads introduce creation and scheduling overhead and can contend
for CPU, cache, and memory resources.

Additional edge cases included already-sorted input, reverse-sorted input,
duplicate values, very small arrays, odd-sized ranges, and different random
seeds.

## Known bugs

No known correctness bugs remain in the submitted implementation. Performance
varies with hardware, operating-system scheduling, system load, and the
selected cutoff level.

## Reflection and self-assessment

The main challenge was understanding how ordinary recursive merge sort maps
onto a recursive thread structure. The key distinction is that
`my_mergesort` calls itself directly, while `parallel_mergesort` creates new
threads whose start routine is `parallel_mergesort`.

Synchronization was also important. A parent cannot merge its two ranges
until both child ranges are sorted, so `pthread_join` is required before the
parent calls `merge`. The global arrays `A` and `B` can be used without a
mutex because concurrently running sibling threads work on non-overlapping
index ranges. The parent accesses the combined range only after joining both
children.

Thread argument lifetime required care. `buildArgs` dynamically allocates
argument structures, and the parent frees each child argument only after that
child has completed. Allocation or thread-creation failures fall back to
single-threaded merge sort so that correctness is preserved.

Testing showed that parallelism has a practical limit. Increasing the cutoff
initially produced a large speedup, but beyond the best-performing level,
additional threads did not improve runtime consistently. Thread creation,
scheduling, cache behaviour, and memory bandwidth all introduce overhead.

## Sources used

- COMP2002 assignment specification and supplied starter code.
- POSIX threads documentation for `pthread_create` and `pthread_join`.
- C standard library documentation for `malloc`, `free`, and `memcpy`.
- OpenAI ChatGPT, used for conceptual guidance on merge sort and POSIX
  threads, debugging, testing strategy, and review of the implementation. The
  final code was tested and reviewed by the authors.
