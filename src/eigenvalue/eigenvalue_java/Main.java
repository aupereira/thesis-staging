import java.util.Arrays;
import java.util.random.*;
import java.math.*;

public class Main {
    
    public static double[][] randHermitianMatrix(int r, int c)
    {
            double[][] mat = new double[r][c];
            
            for (int i = 0; i < r; i++) 
            {
                for (int j = 0; j < i; j++)
                {
                    mat[i][j] = Math.random();
                    mat[j][i] = mat[i][j];
                }
                mat[i][i] = Math.random();
            }

            // for (int i = 0; i < r; i++) 
            // {
            //     for (int j = 0; j < c; j++)
            //     {
            //         System.out.print(mat[i][j] + " ");
            //     }
            //     System.out.println();
            // }

            return mat;
    }

    public static double norm(double[] vec)
    {
        double norm = 0.0;
        for (int i = 0; i < vec.length; i++)
        {
            norm += vec[i] * vec[i];
        }
        return Math.sqrt(norm);
    }

    // public static double[][] identityMat(int n)
    // {
    //     double[][] mat = new double[n][n];
        
    //     for (int i = 0; i < n; i++)
    //     {
    //         mat[i][i] = 1;
    //     }

    //     return mat;
    // }

    // public static double[][] transposeMat(double[][] A)
    // {
    //     int r = A.length;
    //     int c = A[0].length;
    //     double[][] mat = new double[c][r];

    //     for (int i = 0; i < r; i++)
    //     {
    //         for (int j = 0; j < c; j++)
    //         {
    //             mat[j][i] = A[i][j];
    //         }
    //     }

    //     return mat;
    // }

    // public static double[][] GEMM(double[][] A, double[][] B)
    // {
    //     int A_i = A.length;
    //     int B_i = B[0].length;
    //     int A_j = A[0].length;

    //     B = transposeMat(B);

    //     double[][] C = new double[A_i][B_i];

    //     for (int i = 0; i < A_i; i++)
    //     {
    //         for (int j = 0; j <B_i; j++)
    //         {
    //             for (int k = 0; k < A_j; k++)
    //             {
    //                 C[i][j] += A[i][k] * B[j][k];
    //             }
    //         }
    //     }

    //     return C;
    // }

    // public static double[][] householderMat(double[] vec)
    // {
    //     int n = vec.length;
    //     double norm = norm(vec);
    //     double[][] mat = identityMat(n);
        
    //     double[][] outerProd = new double[n][n];
    //     for (int i = 0; i < n; i++)
    //     {
    //         for (int j = 0; j < n; j++)
    //         {
    //             outerProd[i][j] = vec[i] * vec[j];
    //         }
    //     }

    //     double factor = 2 / (norm * norm);
    //     //Scalar-Matrix Multiplication
    //     for (int i = 0; i < n; i++)
    //     {
    //         for (int j = 0; j < n; j++)
    //         {
    //             outerProd[i][j] *= factor;
    //         }
    //     }

    //     //Matrix Subtraction
    //     for (int i = 0; i < n; i++)
    //     {
    //         for (int j = 0; j < n; j++)
    //         {
    //             mat[i][j] -= outerProd[i][j];
    //         }
    //     }

    //     return mat;
    // }

    public static double[][] tridiagonalize(double[][] A) {
        int n = A.length;
        double[][] Q = new double[n][n];
        double[][] T = new double[n][n];
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                Q[i][j] = (i == j) ? 1.0 : 0.0;
                T[i][j] = A[i][j];
            }
        }
        for (int k = 0; k < n - 2; k++) {
            double[] x = new double[n - k];
            for (int i = k; i < n; i++) {
                x[i - k] = T[i][k];
            }
            double alpha = Math.signum(x[0]) * norm(x);
            double[] u = new double[n];
            u[k] = x[0] + alpha;
            for (int i = k + 1; i < n; i++) {
                u[i] = x[i - k];
            }
            double beta = norm(u);
            for (int i = k; i < n; i++) {
                u[i] /= beta;
            }
            for (int j = k; j < n; j++) {
                double sum = 0.0;
                for (int i = k; i < n; i++) {
                    sum += u[i] * T[i][j];
                }
                double tau = sum / beta;
                for (int i = k; i < n; i++) {
                    T[i][j] -= tau * u[i];
                }
            }
            for (int j = 0; j < n; j++) {
                double sum = 0.0;
                for (int i = k; i < n; i++) {
                    sum += u[i] * Q[i][j];
                }
                double tau = sum / beta;
                for (int i = k; i < n; i++) {
                    Q[i][j] -= tau * u[i];
                }
            }
        }
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                T[i][j] = (i == j) ? T[i][j] : ((i == j + 1 || i == j - 1) ? T[i][j] : 0.0);
            }
        }
        return T;
    }

    public static void main(String[] args)
    {
        double[][] A = randHermitianMatrix(1024*8, 1024*8);
        // System.out.println();
        double[][] C = tridiagonalize(A);
        // System.out.println();
        // for (int i = 0; i < C.length; i++) 
        // {
        //     for (int j = 0; j < C[0].length; j++)
        //     {
        //         System.out.print(C[i][j] + " ");
        //     }
        //     System.out.println();
        // }
    }

}