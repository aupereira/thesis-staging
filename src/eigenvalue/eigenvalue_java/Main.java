// package eigenvalue_java;

class Main {
    static void printVec(double[] v) {
        System.out.println();
        for (int i = 0; i < v.length; i++) {
            System.out.printf("%f ", v[i]);
        }
        System.out.println();
    }
    
    static void printMat(double[][] A) {
        System.out.println();
        for (int i = 0; i < A.length; i++) {
            System.out.print("[ ");
            for (int j = 0; j < A[0].length; j++) {
                System.out.printf("%f ", A[i][j]);
            }
            System.out.println("]");
        }
        System.out.println();
    }

    static void mCopy(int len, double[][] FROM, double[][] TO) {
        for (int i = 0; i < len; i++) {
            for (int j = 0; j < len; j++) {
                TO[i][j] = FROM[i][j];
            }
        }
    }

    static void rndSymmetric(double[][] A) {
        int len = A.length;

        for (int i = 0; i < len; i++) {
            A[i][i] = Math.random();
            for (int j = i + 1; j < len; j++) {
                A[i][j] = Math.random();
                A[j][i] = A[i][j];
            }
        }
    }

    static double sgn(double x) {
        return x >= 0.0 ? 1.0 : -1.0;
    }

    static double norm2(int len, double[] v) {
        double norm = 0;
        for (int i = 0; i < len; i++) {
            norm += v[i] * v[i];
        }
        return Math.sqrt(norm);
    }

    static void vDivS(int len, double[] v, double s) {
        for (int i = 0; i < len; i++) {
            v[i] /= s;
        }
    }

    static void S2OPAI(int len, double[] v, double[][] A) {
        for (int i = 0; i < len; i++) {
            for (int j = 0; j < len; j++) {
                A[i][j] = -2.0 * v[i] * v[j];
            }
        }

        for (int i = 0; i < len; i++) {
            A[i][i] += 1.0;
        }
    }

    static void matrixMul(int len, double[][] A, double[][] B, double[][] C, double[][] BTMP) {
        for (int i = 0; i < len; i++) {
            for (int j = 0; j < len; j++) {
                BTMP[i][j] = B[j][i];
            }
        }

        for (int i = 0; i < len; i++) {
            for (int j = 0; j < len; j++) {
                C[i][j] = 0;
                for (int k = 0; k < len; k++) {
                    C[i][j] += A[i][k] * BTMP[j][k];
                }
            }
        }
    }

    static void tridiagonalize(int N, double[][] A, double[][] P, double[][] TEMP, double[][] TPOSE, double[] v) {
        double alpha;
        double r;

        for (int k = 0; k < N - 2; k++) {
            for (int j = 0; j < k + 1; j++) {
                v[j] = 0.0;
            }
            for (int j = k + 1; j < N; j++) {
                v[j] = A[j][k];
            }
        
            alpha = -sgn(v[k + 1]) * norm2(N, v);
            r = 2.0 * Math.sqrt(0.5 * ((alpha * alpha) - (alpha * v[k + 1])));

            v[k + 1] = (v[k + 1] - alpha) / r;
            for (int j = k + 2; j < N; j++) {
                v[j] /= r;
            }

            S2OPAI(N, v, P);
            matrixMul(N, P, A, TEMP, TPOSE);
            matrixMul(N, TEMP, P, A, TPOSE);
        }
    }

    static void householder(int len, double[][] A, double[][] A1, double[][] Q, double[][] H, double[][] TEMP, double[][] TPOSE, double[] v) {
        mCopy(len, A, A1);
        double[][] SWAP;

        double s = A1[len - 1][len - 1];
        double t = A1[len - 2][len - 2];
        double x = A1[len - 2][len - 1];
        double d = (t - s) / 2.0;
        double shift = s - x * x / (d + sgn(d) * Math.sqrt(d * d + x * x));

        for (int i = 0; i < len; i++) {
            A1[i][i] -= shift;
        }

        for (int k = 0; k < len - 1; k++) {
            for (int j = 0; j < k; j++) {
                v[j] = 0.0;
            }
            for (int j = k; j < len; j++) {
                v[j] = A1[j][k];
            }

            v[k] += norm2(len, v) * sgn(v[k + 1]);
            vDivS(len, v, norm2(len, v));

            if (k == 0) {
                S2OPAI(len, v, Q);
                matrixMul(len, Q, A1, TEMP, TPOSE);
                SWAP = A1;
                A1 = TEMP;
                TEMP = SWAP;
            }
            else {
                S2OPAI(len, v, H);
                matrixMul(len, H, A1, TEMP, TPOSE);
                SWAP = A1;
                A1 = TEMP;
                TEMP = SWAP;
                matrixMul(len, Q, H, TEMP, TPOSE);
                SWAP = Q;
                Q = TEMP;
                TEMP = SWAP;
            }
        }

        matrixMul(len, A1, Q, A, TPOSE);

        for (int i = 0; i < len; i++) {
            A[i][i] += shift;
        }
    }

    static void eigQR(double[][] A, double[] eigVals, double[][] T1, double[][] T2, double[][] T3, double[][] T4, double[][] T5, double[] v) {
        int N = A.length;

        double currentEigval;
        double prevEigval = Double.MAX_VALUE;

        int ind = N;

        while (ind > 1) {
            boolean flag = true;
            while (flag) {
                // System.out.printf("%d\n", ind); // Useful for QR input size tuning
                householder(ind, A, T1, T2, T3, T4, T5, v);
                
                currentEigval = A[ind - 1][ind - 1];
        
                if (Math.abs(currentEigval - prevEigval) < 1e-10) {
                    eigVals[ind - 1] = currentEigval;
                    
                    flag = false;
                    ind--;
                    
                    prevEigval = A[ind - 1][ind - 1];
                } else {
                    prevEigval = currentEigval;
                }
            }
            eigVals[0] = A[0][0];
        }
    }

    static void eigenLoop(int size, int loops) {
        double[][] A = new double[size][size];

        double[][] T1 = new double[size][size];
        double[][] T2 = new double[size][size];
        double[][] T3 = new double[size][size];
        double[][] T4 = new double[size][size];
        double[][] T5 = new double[size][size];
        double[] v = new double[size];

        double[] eigvals = new double[size];

        for (int i = 0; i < loops; i++) {
            rndSymmetric(A);
            tridiagonalize(size, A, T1, T2, T3, v);
            eigQR(A, eigvals, T1, T2, T3, T4, T5, v);
        }
    }

    public static void main(String[] args) {
        if (args.length != 3) {
            System.out.println("Usage: java Main <matrix size> <num loops> <num threads>");
        }

        int mSize = Integer.parseInt(args[0]);
        int numLoops = Integer.parseInt(args[1]);
        int numThreads = Integer.parseInt(args[2]);

        Thread[] threads = new Thread[numThreads];

        for (int i = 0; i < numThreads; i++) {
            threads[i] = new Thread(() -> eigenLoop(mSize, numLoops));
        }

        for (int i = 0; i < numThreads; i++) {
            threads[i].start();
        }

        for (int i = 0; i < numThreads; i++) {
            try {
                threads[i].join();
            } catch (InterruptedException e) {
                System.out.println("Thread interrupted");
            }
        }

        // int mSize = 4;
        // double[][] A = {{4.0, 1.0, -2.0, 2.0}, {1.0, 2.0, 0.0, 1.0}, {-2.0, 0.0, 3.0, -2.0}, {2.0, 1.0, -2.0, -1.0}};

        // double[][] T1 = new double[mSize][mSize];
        // double[][] T2 = new double[mSize][mSize];
        // double[][] T3 = new double[mSize][mSize];
        // double[][] T4 = new double[mSize][mSize];
        // double[][] T5 = new double[mSize][mSize];
        // double[] v = new double[mSize];

        // double[] eigvals = new double[mSize];

        // tridiagonalize(mSize, A, T1, T2, T3, v);
        // eigQR(A, eigvals, T1, T2, T3, T4, T5, v);

        // printVec(eigvals);
    }
}