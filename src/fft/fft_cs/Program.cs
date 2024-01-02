using System;
using System.Numerics;
using System.Threading;

class Program {
    static int IntPow(int b, int exp) {
        int result = 1;
        while (exp > 0) {
            result *= b;
            exp--;
        }
        return result;
    }

    static Complex RandComplex(Random rnd) {
        return new Complex(rnd.NextDouble(), rnd.NextDouble());
    }

    static void FFT(ref Complex[] x) {
        int n = x.Length;

        if (n <= 1) {
            return;
        }

        Complex[] even = new Complex[n / 2];
        Complex[] odd = new Complex[n / 2];

        for (int i = 0; i < n / 2; i++) {
            even[i] = x[2 * i];
            odd[i] = x[2 * i + 1];
        }

        FFT(ref even);
        FFT(ref odd);

        for (int k = 0; k < n / 2; k++) {
            Complex t = Complex.Exp(-2.0 * Math.PI * k / n * Complex.ImaginaryOne) * odd[k];
            x[k] = even[k] + t;
            x[k + n / 2] = even[k] - t;
        }
    }

    static void FFTLoop(int size, int loops) {
        Complex[] x = new Complex[size];
        Random rand = new Random();

        for (int loop = 0; loop < loops; loop++) {
            for (int i = 0; i < size; i++) {
                x[i] = RandComplex(rand);
            }
            FFT(ref x);
        }
        return;
    }

    static void Main(string[] args) {
        if (args.Length != 3) {
            Console.WriteLine("Usage: ./fft_cs <FFT size> <num loops> <num threads>");
            return;
        }

        int FFTSize = IntPow(2, int.Parse(args[0]));
        int numLoops = int.Parse(args[1]);
        int numThreads = int.Parse(args[2]);

        Thread[] threads = new Thread[numThreads];

        for (int i = 0; i < numThreads; i++) {
            threads[i] = new Thread(() => FFTLoop(FFTSize, numLoops));
            threads[i].Start();
        }

        foreach (Thread thread in threads) {
            thread.Join();
        }
    }
}