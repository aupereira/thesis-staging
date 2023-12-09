package eigenvalue_java;

class Main {
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
    
    static double[][] identity(int n) {
        double[][] I = new double[n][n];
        for (int i = 0; i < n; i++) {
            I[i][i] = 1;
        }
        return I;
    }

    static double DNRM2(double[] A) {
        double norm = 0;
        for (int i = 0; i < A.length; i++) {
            norm += A[i] * A[i];
        }
        return Math.sqrt(norm);
    }

    static double[] stdBasis(int len, int pos) {
        double[] e = new double[len];
        e[pos] = 1;
        return e;
    }
    
    static double[][] mul2OuterProd(double[] v) {
        int I = v.length;
        double[][] A = new double[I][I];
        
        for (int i = 0; i < I; i++) {
            for (int j = 0; j < I; j++) {
                A[i][j] = -2.0 * v[i] * v[j];
            }
        }
        
        return A;
    }
    
    static double[] vAdd(double[] v1, double[] v2) {
        double[] w = new double[v1.length];
        for (int i = 0; i < v1.length; i++) {
            w[i] = v1[i] + v2[i];
        }
        return w;
    }

    static double[] vMulS(double[] v, double s) {
        double[] w = new double[v.length];
        for (int i = 0; i < v.length; i++) {
            w[i] = v[i] * s;
        }
        return w;
    }

    static void vDivS(double[] v, double s) {
        for (int i = 0; i < v.length; i++) {
            v[i] /= s;
        }
    }

    static double[][] mAdd(double[][] A, double[][] B) {
        int I = A.length;
        int J = A[0].length;
        
        for (int i = 0; i < I; i++) {
            for (int j = 0; j < J; j++) {
                A[i][j] += B[i][j];
            }
        }

        return A;
    }

    static double[][] mSub(double[][] A, double[][] B) {
        int I = A.length;
        int J = A[0].length;
        
        for (int i = 0; i < I; i++) {
            for (int j = 0; j < J; j++) {
                A[i][j] -= B[i][j];
            }
        }

        return A;
    }
    
    static double sgn(double x) {
        if (x >= 0.0) {
            return 1.0;
        }
        else {
            return -1.0;
        }
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

    static void transposeIP(double[][] A) {
        int n = A.length;
        int m = A[0].length;
        
        double temp;

        for (int i = 0; i < n; i++) {
            for (int j = i; j < m; j++) {
                temp = A[i][j];
                A[i][j] = A[j][i];
                A[j][i] = temp;
            }
        }

        return;
    }

    static double[][] GEMM(double[][] A, double[][] B) {
        double t_B[][] = transpose(B);
        
        int I = A.length;
        int J = B.length;
        int K = A[0].length;

        double[][] C = new double[I][J];

        for (int i = 0; i < I; i++) {
            for (int j = 0; j < J; j++) {
                for (int k = 0; k < K; k++) {
                    C[i][j] += A[i][k] * t_B[j][k];
                }
            }
        }

        return C;
    }

    static double[][] householder(double[][] R) {
        int n = R.length;
        int m = R[0].length;

        double[][] Q = new double[n][n];
        double[][] H;
        double[] v;

        for (int k = 0; k < m - 1; k++) {
            v = new double[n];
            for (int j = k; j < n; j++) {
                v[j] = R[j][k];
            }

            v = vAdd(v, vMulS(stdBasis(n, k), DNRM2(v) * sgn(v[k + 1])));
            vDivS(v, DNRM2(v));
            H = mAdd(identity(n), mul2OuterProd(v));
            R = GEMM(H, R);
            if (k == 0) {
                Q = H;
            } else {
                Q = GEMM(Q, H);
            }
        }
        
        return R;
    }

    static double[][] tridiagonalize(double[][] A) {
        int n = A.length;
        int m = A[0].length;

        double alpha;
        double r;
        double[] v;

        double[][] P = new double[n][n];

        for (int k = 0; k < m - 2; k++) {
            v = new double[n];
            for (int j = k + 1; j < n; j++) {
                v[j] = A[j][k];
            }
            
            alpha = -sgn(v[k + 1]) * DNRM2(v);
            r = 2.0 * Math.sqrt(0.5 * ((alpha * alpha) - (alpha * v[k + 1])));

            v[k + 1] = (v[k + 1] - alpha) / (r);
            for (int j = k + 2; j < n; j++) {
                v[j] = v[j] / r; 
            }

            P = mAdd(identity(n), mul2OuterProd(v));
            A = GEMM(GEMM(P, A), P);
        }

        return A;
    }

    public static void main(String[] args) {
        int iter;
        int size;

        if (args.length == 2) {
            iter = Integer.parseInt(args[0]);
            size = Integer.parseInt(args[1]);
            System.out.println("Running " + iter + " iterations of size " + size);
        } else {
            iter = 64;
            size = 256;
            System.out.println("To specify iterations and size, run:\n    java Main <iterations> <size>");
            System.out.println("Proceeding with default values: %d iterations of size %d".formatted(iter, size));
        }

        for (int i = 0; i < iter; i++) {
            tridiagonalize(rndSymmetric(size));
            System.out.println(i);
        }

        // Basic validation test for tridiagonalization algorithm
        // double[][] test = {{4.0, 1.0, -2.0, 2.0}, {1.0, 2.0, 0.0, 1.0}, {-2.0, 0.0, 3.0, -2.0}, {2.0, 1.0, -2.0, -1.0}};
        // printMat(tridiagonalize(test));
    }
}