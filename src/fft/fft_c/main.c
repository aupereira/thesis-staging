#include <stdio.h>
#include <stdlib.h>
#include <complex.h>
#include <time.h>

#ifdef _WIN64
#include <windows.h>
#endif

#ifdef __linux__
#include <pthread.h>
#endif

#if _MSC_VER
#define dComplex _Dcomplex

_Dcomplex _Caddcc(_Dcomplex a, _Dcomplex b) {
    double real = creal(a) + creal(b);
    double imag = cimag(a) + cimag(b);
    return _Cbuild(real, imag);
}

_Dcomplex _Csubcc(_Dcomplex a, _Dcomplex b) {
    double real = creal(a) - creal(b);
    double imag = cimag(a) - cimag(b);
    return _Cbuild(real, imag);
}

#else
#define dComplex double complex
#endif

#define M_PI 3.14159265358979323846

int fftSize, numLoops;

dComplex randComplex() {
    #if _MSC_VER
    return _Cbuild((double)rand()/RAND_MAX, (double)rand()/RAND_MAX);
    #else
    return (double)rand()/RAND_MAX, (double)rand()/RAND_MAX*I;
    #endif
}

void fft(dComplex *x, int N) {
    if (N == 1) {
        return;
    }

    dComplex *even = malloc(N/2 * sizeof(dComplex));
    dComplex *odd = malloc(N/2 * sizeof(dComplex));

    for (int i = 0; i < N/2; i++) {
        even[i] = x[2*i];
        odd[i] = x[2*i+1];
    }

    fft(even, N/2);
    fft(odd, N/2);

    for (int k = 0; k < N/2; k++) {
        #if _MSC_VER
        dComplex t = _Cmulcc(cexp(_Cmulcc(_Cbuild(-2.0 * M_PI * k / N, 0),_Cbuild(0.0, 1.0))), odd[k]);
        x[k] = _Caddcc(even[k], t);
        x[k + N / 2] = _Csubcc(even[k], t);
        #else
        dComplex t = cexp(-2.0 * M_PI * I * k / N) * odd[k];
        x[k] = even[k] + t;
        x[k + N / 2] = even[k] - t;
        #endif
    }

    free(even);
    free(odd);
}

void fftLoop(int size, int loops) {
    srand(time(NULL));

    dComplex *x = malloc(size * sizeof(dComplex));
    
    for (int loop = 0; loop < loops; loop++) {
        for (int i = 0; i < size; i++) {
            x[i] = randComplex();
        }
        fft(x, size);
    }

    free(x);
}

#ifdef _WIN64
DWORD WINAPI WinThreadProc(LPVOID lpParam) {
    fftLoop(fftSize, numLoops);
    return 0;
}
#endif

#ifdef __linux__
void* linuxThreadCaller(void* args) {
    fftLoop(fftSize, numLoops);
    return NULL;
}
#endif

int main(int argc, char *argv[]) {
    if (argc != 4) {
        printf("Usage: ./fft_c <FFT size> <num loops> <num threads>");
        return 1;
    }

    fftSize = 1 << atoi(argv[1]);
    numLoops = atoi(argv[2]);
    int numThreads = atoi(argv[3]);

    #ifdef _WIN64
        HANDLE* hThreads = malloc(numThreads * sizeof(HANDLE));

        for (int i = 0; i < numThreads; i++) {
            hThreads[i] = CreateThread(
                NULL,
                0,
                WinThreadProc,
                NULL,
                0,
                NULL);

            if (hThreads[i] == NULL) {
                printf("CreateThread failed (%lu)\n", GetLastError());
                return 1;
            }
        }

        WaitForMultipleObjects(numThreads, hThreads, TRUE, INFINITE);

        for (int i = 0; i < numThreads; i++) {
            CloseHandle(hThreads[i]);
        }

        free(hThreads);
    #endif

    #ifdef __linux__
        pthread_t* threads = malloc(numThreads * sizeof(pthread_t));

        for (int i = 0; i < numThreads; i++) {
            pthread_create(&threads[i], NULL, linuxThreadCaller, NULL);
        }

        for (int i = 0; i < numThreads; i++) {
            pthread_join(threads[i], NULL);
        }

        free(threads);
    #endif

    return 0;
}