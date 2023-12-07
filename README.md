<h2> Staging area for 499Y/T </h2>

This repo contains work in progress versions of the benchmarks used for my thesis. Some things are broken, some things are incomplete, and some things work just fine but will probably be revised in the later stages.

<h3> Current Benchmarks </h3>
1. SHA256
    * Currently functional - produces correct hashes on very large files (tested up to 28 GB)
    * Memory management needs revision (lots of time in malloc)
    * Needs large file (it's not that expensive to compute) - planned ~30-50 GB
    * May switch to a two-threaded model with double buffering to limit time waiting to load files
2. FFT
    * Utilizes a Radix-2 DIT FFT algorithm
    * Almost works correctly
    * Rust has some odd performance behaviors when choosing between Vec and Array types. Need to investigate that further.
    * Rust also gets handily beat by C# in this test.
3. Eigenvalue
    * Probably going to use Householder reflectors and QR decomposition
    * I'm working on it (I've spent quite a few hours learning linalg for this)
4. DataTransform
    * Planned features (though highly dependent on how intensive it is)
    * Some sorting algorithms, including sorting by string
    * 
5. Neural Net
    * Probably an MLP
    * Likely will borrow from prior dataset.