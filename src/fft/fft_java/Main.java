class Main {
  static void fft(Complex[] x) {
    int n = x.length;

    if (n <= 1) {
      return;
    }

    Complex[] even = new Complex[n / 2];
    Complex[] odd = new Complex[n / 2];

    for (int i = 0; i < n / 2; i++) {
      even[i] = x[2 * i];
      odd[i] = x[2 * i + 1];
    }

    fft(even);
    fft(odd);

    for (int k = 0; k < n / 2; k++) {
      Complex t =
          Complex.exp(Complex.fromReal(-2.0 * Math.PI * k / n).mul(Complex.imag())).mul(odd[k]);
      x[k] = even[k].add(t);
      x[k + n / 2] = even[k].sub(t);
    }
  }

  public static void fftLoop(int size, int loops) {
    Complex[] x = new Complex[size];

    for (int loop = 0; loop < loops; loop++) {
      for (int i = 0; i < size; i++) {
        x[i] = Complex.randComplex();
      }
      fft(x);
    }
  }

  public static void main(String[] args) {
    if (args.length != 3) {
      System.out.println("Usage: java Main <FFT size> <num loops> <num threads>");
      return;
    }

    int fftSize = 1 << Integer.parseInt(args[0]);
    int numLoops = Integer.parseInt(args[1]);
    int numThreads = Integer.parseInt(args[2]);

    Thread[] threads = new Thread[numThreads];

    for (int i = 0; i < numThreads; i++) {
      threads[i] = new Thread(() -> fftLoop(fftSize, numLoops));
      threads[i].start();
    }

    for (int i = 0; i < numThreads; i++) {
      try {
        threads[i].join();
      } catch (InterruptedException e) {
        System.out.println("Thread interrupted");
      }
    }
  }
}
