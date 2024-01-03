#include <iostream>
#include <string>
#include <complex>
#include <ctime>
#include <vector>
#include <thread>

#define M_PI 3.14159265358979323846

std::complex<double> randComplex() {
    return std::complex<double>(rand() / (double)RAND_MAX, rand() / (double)RAND_MAX);
}

void fft(std::vector<std::complex<double>> &x) {
    int n = x.size();

    if (n <= 1) {
        return;
    }

    std::vector<std::complex<double>> even(n / 2);
    std::vector<std::complex<double>> odd(n / 2);

    for (int i = 0; i < n / 2; i++) {
        even[i] = x[i * 2];
        odd[i] = x[i * 2 + 1];
    }

    fft(even);
    fft(odd);

    for (int k = 0; k < n / 2; k++) {
        std::complex<double> t = std::exp(-2.0 * M_PI * double(k) / double(n) * std::complex<double>(0,1)) * odd[k];
        x[k] = even[k] + t;
        x[k + n / 2] = even[k] - t;
    }
}

void fftLoop(int size, int loops) {
    std::vector<std::complex<double>> x(size);

    for (int loop; loop < loops; loop++) {
        for (int i = 0; i < size; i++) {
            x[i] = randComplex();
        }
        fft(x);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        std::cout << "Usage: ./fft_cpp <FFT size> <num loops> <num threads>" << std::endl;
        exit(1);
    }
    
    auto fftSize = 1 << std::stoi(argv[1]);
    auto numLoops = std::stoi(argv[2]);
    auto numThreads = std::stoi(argv[3]);

    srand(time(0));

    std::vector<std::thread> threads(numThreads);

    for (auto& thread : threads) {
        thread = std::thread(fftLoop, fftSize, numLoops);
    }

    for (auto& thread : threads) {
        thread.join();
    }

    exit(0);
}