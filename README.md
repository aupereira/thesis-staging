## Staging area for 499Y/T

This repo contains work in progress versions of the benchmarks used for my thesis. Some things are incomplete, and some things work just fine but will be revised in later stages. Currently this only stores reference implementations of benchmarks. Some preliminary translations exist in other langauges, but they are omitted as the reference isn't finalized.

### Current Benchmarks
1. #### SHA256
  * Currently functional - produces correct hashes on very large files (tested up to 28 GB)
  * Needs large file (it's not that expensive to compute) - planned ~30-50 GB
  * Planned switch to a two-threaded model with double buffering to limit time waiting to load files
2. #### FFT
  * Utilizes a Radix-2 DIT FFT algorithm
  * Works correctly
  - Rust has some odd performance behaviors when choosing between Vec and Array types. Need to investigate that further
  - The Rust implementation is currently significantly front-end bound (~35%)
3. #### Eigenvalue
  * Converts real symmetric matrices to tridiagonal form for eigenvalue calculation
  * Tridiagonalization is functional
  * Threading model still up in the air depending on results
4. #### DataTransform
  * Planned features (though highly dependent on how intensive it is)
  * Some sorting algorithms, including sorting by string
  * Indexing by column
5. #### Neural Net
  * Planned to be an MLP


### Basic Usage
1. #### SHA256
  * ./sha256 `<"FILEPATH">`
2. #### FFT
  * ./fft `<FFT_SIZE>` `<LOOP_ITERATIONS>` `<THREADS>`
  * FFT_SIZE - Represents a power of 2. I.e., 16 represents an FFT of size 65,536
  * LOOP_ITERATIONS - The number of FFTs to process.
  * THREADS - Number of worker threads to spawn.
3. #### Eigenvalue
  * java Main `<LOOP_ITERATIONS>` `<MATRIX_SIZE>`
  * LOOP_ITERATIONS - The number of matrices to generate and process.
  * MATRIX_SIZE - The size of the matrices (n x n) to generate and process.