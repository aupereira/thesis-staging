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
    
    static double[][] rndSymmetric(int n) {
        double[][] A = new double[n][n];

        for (int i = 0; i < n; i++) {
            A[i][i] = Math.random();
            for (int j = i + 1; j < n; j++) {
                A[i][j] = Math.random();
                A[j][i] = A[i][j];
            }
        }
        
        return A;
    }
    
    static double sgn(double x) {
        return x >= 0.0 ? 1.0 : -1.0;
    }

    static double norm2(double[] v) {
        double norm = 0;
        for (int i = 0; i < v.length; i++) {
            norm += v[i] * v[i];
        }
        return Math.sqrt(norm);
    }

    static void vDivS(double[] v, double s) {
        for (int i = 0; i < v.length; i++) {
            v[i] /= s;
        }
    }
    
    static double[][] S2OPAI(double[] v) {
        int len = v.length;
        double[][] A = new double[len][len];
        
        for (int i = 0; i < len; i++) {
            for (int j = 0; j < len; j++) {
                A[i][j] = -2.0 * v[i] * v[j];
            }
        }

        for (int i = 0; i < len; i++) {
            A[i][i] += 1.0;
        }
        
        return A;
    }

    static double[][] transpose(double[][] A) {
        int n = A.length;
        int m = A[0].length;

        double[][] B = new double[m][n];

        for (int i = 0; i < n; i++) {
            for (int j = 0; j < m; j++) {
                B[j][i] = A[i][j];
            }
        }

        return B;
    }

    static double[][] matrixMul(double[][] A, double[][] B) {
        double BTMP[][] = transpose(B);
        
        int len = A.length;

        double[][] C = new double[len][len];

        for (int i = 0; i < len; i++) {
            for (int j = 0; j < len; j++) {
                for (int k = 0; k < len; k++) {
                    C[i][j] += A[i][k] * BTMP[j][k];
                }
            }
        }

        return C;
    }

    static double[][] tridiagonalize(double[][] A) {
        int N = A.length;

        double alpha;
        double r;
        double[] v = new double[N];

        double[][] P = new double[N][N];

        for (int k = 0; k < N - 2; k++) {
            for (int j = 0; j < k + 1; j++) {
                v[j] = 0.0;
            }
            for (int j = k + 1; j < N; j++) {
                v[j] = A[j][k];
            }
            
            alpha = -sgn(v[k + 1]) * norm2(v);
            r = 2.0 * Math.sqrt(0.5 * ((alpha * alpha) - (alpha * v[k + 1])));

            v[k + 1] = (v[k + 1] - alpha) / (r);
            for (int j = k + 2; j < N; j++) {
                v[j] = v[j] / r;
            }

            P = S2OPAI(v);
            A = matrixMul(matrixMul(P, A), P);
        }

        return A;
    }

    static double[][] householder(double[][] A) {
        int N = A.length;

        double[][] Q = new double[N][N];
        double[][] H;
        double[] v = new double[N];

        double s = A[N - 1][N - 1];
        double t = A[N - 2][N - 2];
        double x = A[N - 2][N - 1];
        double d = (t - s) / 2;
        double shift = s - x * x / (d + sgn(d) * Math.sqrt(d * d + x * x));

        for (int i = 0; i < N; i++) {
            A[i][i] -= shift;
        }

        for (int k = 0; k < N - 1; k++) {
            for (int j = 0; j < k; j++) {
                v[j] = 0.0;
            }
            for (int j = k; j < N; j++) {
                v[j] = A[j][k];
            }

            v[k] += norm2(v) * sgn(v[k + 1]);
            vDivS(v, norm2(v));
            H = S2OPAI(v);
            A = matrixMul(H, A);
            if (k == 0) {
                Q = H;
            } else {
                Q = matrixMul(Q, H);
            }

        }
        
        A = matrixMul(A, Q);
        
        for (int i = 0; i < N; i++) {
            A[i][i] += shift;
        }
                
        return A;
    }

    static double[] eigQR(double[][] A) {
        int ind = A.length;

        double[] vals = new double[ind];

        double currentEigval;
        double prevEigval = Double.MAX_VALUE;

        while (ind > 1) {
            // System.out.printf("%d\n", ind); // Useful for QR input size tuning
            A = householder(A);
            
            currentEigval = A[ind - 1][ind - 1];
    
            if (Math.abs(currentEigval - prevEigval) < 1e-10) {
                vals[ind - 1] = currentEigval;
                
                ind--;
                
                double[][] temp = new double[ind][ind];
                for (int i = 0; i < temp.length; i++) {
                    for (int j = 0; j < temp[0].length; j++) {
                        temp[i][j] = A[i][j];
                    }
                }
                A = temp;
                
                prevEigval = A[ind - 1][ind - 1];
            } else {
                prevEigval = currentEigval;
            }
        }

        vals[0] = A[0][0];

        return vals;
    }

    public static void eigenLoop(int size, int loops) {
        for (int i = 0; i < loops; i++) {
            eigQR(tridiagonalize(rndSymmetric(size)));
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

        // // Basic validation test
        // double[][] test = {{4.0, 1.0, -2.0, 2.0}, {1.0, 2.0, 0.0, 1.0}, {-2.0, 0.0, 3.0, -2.0}, {2.0, 1.0, -2.0, -1.0}};

        // test = tridiagonalize(test);    
        // double[] eigvals = eigQR(test);
        // printVec(eigvals);
    }
}