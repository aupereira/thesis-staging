package main

import (
	"fmt"
	"math"
	"math/rand"
	"math/cmplx"
	"os"
	"strconv"
	"sync"
)

func intPow(base int64, exp int64) int64 {
	result := int64(1)
	for exp > 0 {
		result *= base
		exp--
	}
	return result
}

func randComplex() complex128 {
	return complex(rand.Float64(), rand.Float64())
}

func fft(x *[]complex128) {
	n := len(*x)

	if n <= 1 {
		return
	}

	even := make([]complex128, n/2)
	odd := make([]complex128, n/2)

	for i := 0; i < n/2; i++ {
		even[i] = (*x)[2*i]
		odd[i] = (*x)[2*i+1]
	}

	fft(&even)
	fft(&odd)

	for k := 0; k < n/2; k++ {
		t := cmplx.Exp(-2 * math.Pi * complex(float64(k), 0) * complex(0, 1) / complex(float64(n), 0)) * odd[k]
		(*x)[k] = even[k] + t
		(*x)[k+n/2] = even[k] - t
	}
}

func fftLoop(size int64, loops int64) {
	x := make([]complex128, size)

	for loop := int64(0); loop < loops; loop++ {
		for i := range x {
			x[i] = randComplex()
		}
	}

	return
}

func main() {

	if len(os.Args) != 4 {
		fmt.Println("Usage: ./fft_go <fft_size> <num_loops> <num_threads>")
		return
	}

	args := make([]int64, 3)
	for i := 0; i < 3; i++ {
		arg, err := strconv.ParseInt(os.Args[i+1], 10, 64)
		if err != nil {
			panic(err)
		}
		args[i] = arg
	}
	
	args[0] = intPow(int64(2), args[0])

	var wg sync.WaitGroup
	wg.Add(int(args[2]))

	for i := int64(0); i < args[2]; i++ {
		go func() {
			fftLoop(args[0], args[1])
			wg.Done()
		}()
	}

	wg.Wait()
}