use rand::Rng;
use std::mem;
use std::thread;
use std::env;

fn print_vec(v: &Vec<f64>) {
    println!("{:?}", v);
}

fn print_mat(A: &Vec<Vec<f64>>) {
    println!();
    for i in 0..A.len() {
        print_vec(&A[i]);
    }
}

fn sgn(x: f64) -> f64 {
    return if x >= 0.0 {1.0} else {-1.0};
}

fn rand_symmetric(n: usize, rng: &mut rand::rngs::ThreadRng, A: &mut Vec<Vec<f64>>) {
    // let mut rng = rand::thread_rng();

    for i in 0..n {
        for j in i+1..n {
            A[i][j] = rng.gen();
            A[j][i] = A[i][j];
        }
    }
}

fn matrix_multiply(n: usize, A: &Vec<Vec<f64>>, B: &Vec<Vec<f64>>, C: &mut Vec<Vec<f64>>, BTMP: &mut Vec<Vec<f64>>) {
    for i in 0..n {
        for j in 0..n {
            BTMP[i][j] = B[j][i];
        }
    }

    for i in 0..n {
        for j in 0..n {
            C[i][j] = 0.0;
            for k in 0..n {
                C[i][j] += A[i][k] * BTMP[j][k];
            }
        }
    }
}

fn norm2(n: usize, v: &Vec<f64>) -> f64 {
    let mut sum: f64 = 0.0;

    for i in 0..n {
        sum += v[i] * v[i];
    }

    return sum.sqrt();
}

fn S2OPAI(len: usize, v: &Vec<f64>, A: &mut Vec<Vec<f64>>) {
    for i in 0..len {
        for j in 0..len {
            A[i][j] = -2.0 * v[i] * v[j];
        }
    }

    for i in 0..len {
        A[i][i] += 1.0;
    }
}

fn v_div_s(len: usize, v: &mut Vec<f64>, s: f64) {
    for i in 0..len {
        v[i] /= s;
    }
}

fn tridiagonalize(n: usize, A: &mut Vec<Vec<f64>>, P: &mut Vec<Vec<f64>>, TEMP: &mut Vec<Vec<f64>>, TPOSE: &mut Vec<Vec<f64>>, v: &mut Vec<f64>) {
    let mut alpha: f64;
    let mut r: f64;

    for k in 0..n-2 {
        for j in 0..k+1 {
            v[j] = 0.0;
        }
        for j in k+1..n {
            v[j] = A[j][k];
        }

        alpha = -sgn(v[k + 1]) * norm2(n, &v);
        r = -2.0 * (0.5 * ((alpha * alpha) - (alpha * v[k + 1]))).sqrt();

        v[k + 1] = (v[k + 1] - alpha) / r;
        for j in k+2..n {
            v[j] /= r;
        }

        S2OPAI(n, &v, P);
        matrix_multiply(n, P, A, TEMP, TPOSE);
        matrix_multiply(n, TEMP, P, A, TPOSE);
    }
}

fn householder(len: usize, A: &mut Vec<Vec<f64>>, A1: &mut Vec<Vec<f64>>, Q: &mut Vec<Vec<f64>>, H: &mut Vec<Vec<f64>>, TEMP: &mut Vec<Vec<f64>>, TPOSE: &mut Vec<Vec<f64>>, v: &mut Vec<f64>) {
    *A1 = A.clone();
    
    let s: f64 = A1[len - 1][len - 1];
    let t: f64 = A1[len - 2][len - 2];
    let x: f64 = A1[len - 2][len - 1];
    let d: f64 = (t - s) / 2.0;
    let shift: f64 = s - (x * x) / (d + sgn(d) * (d * d + x * x).sqrt());

    for i in 0..len {
        A1[i][i] -= shift;
    }

    for k in 0..len-1 {
        for j in 0..k {
            v[j] = 0.0;
        }
        for j in k..len {
            v[j] = A1[j][k];
        }

        v[k] += norm2(len, v) * sgn(v[k + 1]);
        v_div_s(len, v, norm2(len, v));

        if k == 0 {
            S2OPAI(len, &v, Q);
            matrix_multiply(len, Q, A1, TEMP, TPOSE);
            mem::swap(A1, TEMP);
        } else {
            S2OPAI(len, &v, H);
            matrix_multiply(len, H, A1,  TEMP, TPOSE);
            mem::swap(A1, TEMP);
            matrix_multiply(len, Q, H, TEMP, TPOSE);
            mem::swap(Q, TEMP);
        }
    }

    matrix_multiply(len, A1, Q, A, TPOSE);

    for i in 0..len {
        A[i][i] += shift;
    }
}

fn eig_QR(A: &mut Vec<Vec<f64>>, eig_vals: &mut Vec<f64>, T1: &mut Vec<Vec<f64>>, T2: &mut Vec<Vec<f64>>, T3: &mut Vec<Vec<f64>>, T4: &mut Vec<Vec<f64>>, T5: &mut Vec<Vec<f64>>, v: &mut Vec<f64>) {
    let mut ind: usize = A.len();

    let mut current_eigval: f64;
    let mut prev_eigval: f64 = std::f64::MAX;

    while ind > 1 {
        householder(ind, A, T1, T2, T3, T4, T5, v);

        current_eigval = A[ind - 1][ind - 1];

        if (current_eigval - prev_eigval).abs() < 1e-10 {
            eig_vals[ind - 1] = current_eigval;
            
            ind -= 1;

            prev_eigval = A[ind - 1][ind - 1];
        }
        else {
            prev_eigval = current_eigval;
        }
    }

    eig_vals[0] = A[0][0];
}

fn eigen_loop(size: usize, loops: usize) {
    let mut rng = rand::thread_rng();

    let mut A: Vec<Vec<f64>> = vec![vec![0.0; size]; size];
 
    let mut T1: Vec<Vec<f64>> = vec![vec![0.0; size]; size];
    let mut T2: Vec<Vec<f64>> = vec![vec![0.0; size]; size];
    let mut T3: Vec<Vec<f64>> = vec![vec![0.0; size]; size];
    let mut T4: Vec<Vec<f64>> = vec![vec![0.0; size]; size];
    let mut T5: Vec<Vec<f64>> = vec![vec![0.0; size]; size];
    let mut v: Vec<f64> = vec![0.0; size];

    let mut eig_vals: Vec<f64> = vec![0.0; size];
    
    for _ in 0..loops {
        rand_symmetric(size, &mut rng, &mut A);
        tridiagonalize(size, &mut A, &mut T1, &mut T2, &mut T3, &mut v);
        eig_QR(&mut A, &mut eig_vals, &mut T1, &mut T2, &mut T3, &mut T4, &mut T5, &mut v);
    }
}

fn main() {
    let args: Vec<String> = env::args().collect();

    if args.len() != 4 {
        println!("Usage: ./eigenvalue_rs <fft_size> <num_loops> <num_threads>");
        return;
    }

    let m_size: usize = args[1].parse::<usize>().unwrap();
    let num_loops: usize = args[2].parse::<usize>().unwrap();
    let num_threads: usize = args[3].parse::<usize>().unwrap();

    let handles: Vec<_> = (0..num_threads)
        .map(|_| {
            let size: usize = m_size.clone();
            let loops: usize = num_loops.clone();
            thread::spawn(move || {
                eigen_loop(size, loops);
            })
        })
        .collect();

    for handle in handles {
        handle.join().unwrap();
    }

    // let mut m_size: usize = 4;

    // let mut A: Vec<Vec<f64>> = vec![
    //     vec![4.0, 1.0, -2.0, 2.0],
    //     vec![1.0, 2.0, 0.0, 1.0],
    //     vec![-2.0, 0.0, 3.0, -2.0],
    //     vec![2.0, 1.0, -2.0, -1.0]
    // ];

    // let mut T1: Vec<Vec<f64>> = vec![vec![0.0; m_size]; m_size];
    // let mut T2: Vec<Vec<f64>> = vec![vec![0.0; m_size]; m_size];
    // let mut T3: Vec<Vec<f64>> = vec![vec![0.0; m_size]; m_size];
    // let mut T4: Vec<Vec<f64>> = vec![vec![0.0; m_size]; m_size];
    // let mut T5: Vec<Vec<f64>> = vec![vec![0.0; m_size]; m_size];
    // let mut v: Vec<f64> = vec![0.0; m_size];

    // let mut eig_vals: Vec<f64> = vec![0.0; m_size];
    
    // tridiagonalize(m_size, &mut A, &mut T1, &mut T2, &mut T3, &mut v);
    // eig_QR(&mut A, &mut eig_vals, &mut T1, &mut T2, &mut T3, &mut T4, &mut T5, &mut v);

    // print_vec(&eig_vals);
}