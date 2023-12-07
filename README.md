## Staging area for 499Y/T

This repo contains work in progress versions of the benchmarks used for my thesis. Some things are broken, some things are incomplete, and some things work just fine but will probably be revised in the later stages.

### Current Benchmarks
1. #### SHA256
  * Currently functional - produces correct hashes on very large files (tested up to 28 GB)
  * Memory management needs revision (lots of time in malloc)
  * Needs large file (it's not that expensive to compute) - planned ~30-50 GB
  * May switch to a two-threaded model with double buffering to limit time waiting to load files
2. #### FFT
  * Utilizes a Radix-2 DIT FFT algorithm
  * Works correctly
  - Rust has some odd performance behaviors when choosing between Vec and Array types. Need to investigate that further
  - The Rust implementation is currently significantly front-end bound (~35%)
3. #### Eigenvalue
  * Currently uses Householder reflections, but it's broken.
  * I'm currently working on it (I've spent quite a few hours learning linalg for this)
  * Should follow a different thread model (splitting workload between cores, not just running multiple in parallel).
4. #### DataTransform
  * Planned features (though highly dependent on how intensive it is)
  * Some sorting algorithms, including sorting by string
  * Indexing by column
5. #### Neural Net
  * Currently planned to be an MLP


### Basic Usage
1. #### SHA256
  * ./sha256 `<"FILEPATH">`
2. #### FFT
  * ./fft `<FFT_SIZE>` `<LOOP_ITERATIONS>` `<THREADS>`
  * FFT_SIZE - Represents a power of 2. I.e., 16 represents an FFT of size 65,536
  * LOOP_ITERATIONS - The number of FFTs to process.
  * THREADS - Number of worker threads to spawn.