#include <inttypes.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <float.h> // Maybe remove the DBL_MAX!?

#ifdef _WIN32
#include <Windows.h>
#endif

#ifdef __linux__
#include <pthread.h>
#endif

#define sgn(x) (x >= 0.0 ? 1.0 : -1.0)

int mSize, numLoops, numThreads;

void printVec(size_t N, double v[N]) {
    printf("[");
    for (size_t i = 0; i < N; i++) {
        printf("%f", v[i]);
        if (i != N - 1) {
            printf(", ");
        }
    }
    printf("]\n");
}

void printMat(size_t N, double A[N][N]) {
    for (size_t i = 0; i < N; i++) {
        printf("[");
        for (size_t j = 0; j < N; j++) {
            printf("%f", A[i][j]);
            if (j != N - 1) {
                printf(", ");
            }
        }
        printf("]\n");
    }
}

void mCopy(size_t N, size_t len, double FROM[N][N], double TO[N][N]) {
    for (size_t i = 0; i < len; i++) {
        for (size_t j = 0; j < len; j++) {
            TO[i][j] = FROM[i][j];
        }
    }
}

void rndSymmetric(size_t N, double A[N][N]) {
    for (size_t i = 0; i < N; i++) {
        A[i][i] = rand() / (double)RAND_MAX;
        for (size_t j = i + 1; j < N; j++) {
            A[i][j] = rand() / (double)RAND_MAX;
            A[j][i] = A[i][j];
        }
    }
}

double norm2(size_t N, size_t len, double v[N]) {
    double norm = 0.0;
    for (size_t i = 0; i < len; i++) {
        norm += v[i] * v[i];
    }
    return sqrt(norm);
}

void vDivS(size_t N, size_t len, double v[N], double s) {
    for (size_t i = 0; i < len; i++) {
        v[i] /= s;
    }
}

void S2OPAI(size_t N, size_t len, double v[N], double A[N][N]) {
    for (size_t i = 0; i < len; i++) {
        for (size_t j = 0; j < len; j++) {
            A[i][j] = -2.0 * v[i] * v[j];
        }
    }

    for (size_t i = 0; i < len; i++) {
        A[i][i] += 1.0;
    }
}

void mMul(size_t N, size_t len, double A[N][N], double B[N][N], double C[N][N], double BTMP[N][N]) {
    for (size_t i = 0; i < len; i++) {
        for (size_t j = 0; j < len; j++) {
            BTMP[i][j] = B[j][i];
        }
    }
    
    for (size_t i = 0; i < len; i++) {
        for (size_t j = 0; j < len; j++) {
            C[i][j] = 0.0;
            for (size_t k = 0; k < len; k++) {
                C[i][j] += A[i][k] * BTMP[j][k];
            }
        }
    }
}

void tridiagonalize(size_t N, double A[N][N], double P[N][N], double TEMP[N][N], double TPOSE[N][N], double v[N]) {
    double alpha;
    double r;
    
    for (size_t k = 0; k < N - 2; k++) {
        for (size_t j = 0; j < k + 1; j++) {
            v[j] = 0.0;
        }
        for (size_t j = k + 1; j < N; j++) {
            v[j] = A[j][k];         
        }

        alpha = -sgn(v[k + 1]) * norm2(N, N, v);
        r = 2.0 * sqrt(0.5 * ((alpha * alpha) - (alpha * v[k + 1])));

        v[k + 1] = (v[k + 1] - alpha) / r;
        for (size_t j = k + 2; j < N; j++) {
            v[j] /= r;
        }

        S2OPAI(N, N, v, P);
        mMul(N, N, P, A, TEMP, TPOSE);
        mMul(N, N, TEMP, P, A, TPOSE);
    }
}

void householder(size_t N, size_t len, double A[N][N], double A1[N][N], double Q[N][N], double H[N][N], double TEMP[N][N], double TPOSE[N][N], double v[N]) {
    mCopy(N, len, A, A1);
    double (*SWAP)[N];
    
    double s = A1[len - 1][len - 1];
    double t = A1[len - 2][len - 2];
    double x = A1[len - 2][len - 1];
    double d = (t - s) / 2.0;
    double shift = s - x * x / (d + sgn(d) * sqrt(d * d + x * x));

    for (size_t i = 0; i < len; i++) {
        A1[i][i] -= shift;
    }

    for (size_t k = 0; k < len - 1; k++) {
        for (size_t j = 0; j < k; j++) {
            v[j] = 0.0;
        }
        for (size_t j = k; j < len; j++) {
            v[j] = A1[j][k];         
        }

        v[k] += norm2(N, len, v) * sgn(v[k + 1]);
        vDivS(N, len, v, norm2(N, len, v));

        if (k == 0) {
            S2OPAI(N, len, v, Q);
            mMul(N, len, Q, A1, TEMP, TPOSE);
            SWAP = A1;
            A1 = TEMP;
            TEMP = SWAP;
        }
        else {
            S2OPAI(N, len, v, H);
            mMul(N, len, H, A1, TEMP, TPOSE);
            SWAP = A1;
            A1 = TEMP;
            TEMP = SWAP;
            mMul(N, len, Q, H, TEMP, TPOSE);
            SWAP = Q;
            Q = TEMP;
            TEMP = SWAP;
        }
    }

    mMul(N, len, A1, Q, A, TPOSE);

    for (size_t i = 0; i < len; i++) {
        A[i][i] += shift;
    }
}

void eigQR(size_t N, double A[N][N], double eigvals[N], double T1[N][N], double T2[N][N], double T3[N][N], double T4[N][N], double T5[N][N], double v[N]) {
    double currentEigval;
    double prevEigval = DBL_MAX;

    size_t ind = N;

    while (ind > 1) {
        householder(N, ind, A, T1, T2, T3, T4, T5, v);

        currentEigval = A[ind - 1][ind - 1];

        if (fabs(currentEigval - prevEigval) < 1e-10) {
            // printf("\n%zu", ind - 1); // Useful for QR input size tuning
            eigvals[ind - 1] = currentEigval;
            
            ind--;

            prevEigval = A[ind - 1][ind - 1];
        }
        else {
            prevEigval = currentEigval;
        }
    }

    eigvals[0] = A[0][0];
}

void eigenLoop(size_t size, size_t numLoops) {
    srand(time(NULL));

    double (*A)[size] = malloc(mSize * sizeof *A);

    double (*T1)[size] = malloc(size * sizeof *T1);
    double (*T2)[size] = malloc(size * sizeof *T2);
    double (*T3)[size] = malloc(size * sizeof *T3);
    double (*T4)[size] = malloc(size * sizeof *T4);
    double (*T5)[size] = malloc(size * sizeof *T5);
    double *v = (double*)malloc(size * sizeof(double));

    double *eigvals = (double*)malloc(size * sizeof(double));

    for (size_t i = 0; i < numLoops; i++) {
        rndSymmetric(size, A);
        tridiagonalize(size, A, T1, T2, T3, v);
        eigQR(size, A, eigvals, T1, T2, T3, T4, T5, v);
    }

    free(T1);
    free(T2);
    free(T3);
    free(T4);
    free(v);
    free(eigvals);
}

#ifdef _WIN64
DWORD WINAPI WinThreadProc() {
    eigenLoop(mSize, numLoops);
    return 0;
}
#endif

#ifdef __linux__
void* linuxThreadCaller(void* args) {
    eigenLoop(mSize, numLoops);
    return NULL;
}
#endif

int main(int argc, char *argv[]) {
    if (argc != 4) {
        printf("Usage: ./eigenvalue_c <matrix size> <num loops> <num threads>");
        return 1;
    }

    mSize = atoi(argv[1]);
    numLoops = atoi(argv[2]);
    numThreads = atoi(argv[3]);
    
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

    // // Test case
    // size_t mSize = 4;
    // double (*A)[mSize] = malloc(mSize * sizeof *A);
    // A[0][0] = 4.0;
    // A[0][1] = 1.0;
    // A[0][2] = -2.0;
    // A[0][3] = 2.0;
    // A[1][0] = 1.0;
    // A[1][1] = 2.0;
    // A[1][2] = 0.0;
    // A[1][3] = 1.0;
    // A[2][0] = -2.0;
    // A[2][1] = 0.0;
    // A[2][2] = 3.0;
    // A[2][3] = -2.0;
    // A[3][0] = 2.0;
    // A[3][1] = 1.0;
    // A[3][2] = -2.0;
    // A[3][3] = -1.0;
    
    // double (*T1)[mSize] = malloc(mSize * sizeof *T1);
    // double (*T2)[mSize] = malloc(mSize * sizeof *T2);
    // double (*T3)[mSize] = malloc(mSize * sizeof *T3);
    // double (*T4)[mSize] = malloc(mSize * sizeof *T4);
    // double (*T5)[mSize] = malloc(mSize * sizeof *T5);
    // double *v = (double*)malloc(mSize * sizeof(double));

    // double *eigvals = (double*)malloc(mSize * sizeof(double));

    // tridiagonalize(mSize, A, T1, T2, T3, v);
    // eigQR(mSize, A, eigvals, T1, T2, T3, T4, T5, v);

    // printVec(mSize, eigvals);

    return 0;
}