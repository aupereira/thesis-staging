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
    
    static double[][] rndHermitian(int n) {
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
    
    static void vSub(double[] v1, double[] v2) {
        for (int i = 0; i < v1.length; i++) {
            v1[i] -= v2[i];
        }
    }

    static double[] vMulS(double[] v, double s) {
        for (int i = 0; i < v.length; i++) {
            v[i] *= s;
        }
        return v;
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
    
    static double sgn(double x) {
        if (x >= 0.0) {
            return 1.0;
        }
        else {
            return -1.0;
        }
    }

    static void transpose(double[][] A) {
        int n = A.length;
        int m = A[0].length;
        
        double temp;

        for (int i = 0; i < n; i++) {
            for (int j = 0; j < m; j++) {
                temp = A[i][j];
                A[i][j] = A[j][i];
                A[j][i] = temp;
            }
        }

        return;
    }

    static double[][] GEMM(double[][] A, double[][] B) {
        transpose(B);
        
        int I = A.length;
        int J = B.length;
        int K = A[0].length;

        double[][] C = new double[I][J];

        for (int i = 0; i < I; i++) {
            for (int j = 0; j < J; j++) {
                for (int k = 0; k < K; k++) {
                    C[i][k] += A[i][k] * B[j][k];
                }
            }
        }

        return C;
    }

    static double[][] householder(double[][] A) {
        int n = A.length;
        int m = A[0].length;

        double[][] Q = new double[n][m];
        double[][] R = A;

        for (int i = 0; i < m - 1; i++) {
            double[] v = new double[n];
            for (int j = v.length - i - 1; j >= 0; j--) {
                v[j] = A[i + j][i];
            }
            vSub(v, vMulS(stdBasis(n, i), DNRM2(v)));
            vDivS(v, DNRM2(v));
            double[][] H = mAdd(identity(n), mul2OuterProd(v));
            R = GEMM(H, R);
            if (i == 0) {
                Q = H;
            } else {
                Q = GEMM(Q, H);
            }
        }
        
        return R;
    }

    public static void main(String[] args) {
        double[][] test = rndHermitian(5);
        printMat(test);
        double[][] res = householder(test);
        printMat(res);
    }
}