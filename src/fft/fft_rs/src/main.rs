use num::complex::Complex;
use rand::Rng;
use std::env;
use std::f64::consts::PI;
use std::thread;

fn rng_complex(rng: &mut rand::rngs::ThreadRng) -> Complex<f64> {
    return num::complex::Complex::new(rng.gen_range(0.0..1.0), 0.0);
}

fn fft(x: &mut [Complex<f64>]) {
    let n: usize = x.len();

    if n <= 1 {
        return;
    }

    let mut even: Vec<Complex<f64>> = Vec::new();
    let mut odd: Vec<Complex<f64>> = Vec::new();

    for i in 0..n / 2 {
        even.push(x[2 * i]);
        odd.push(x[2 * i + 1]);
    }

    fft(&mut even);
    fft(&mut odd);

    for k in 0..n / 2 {
        let t = Complex::exp(-2.0 * PI * k as f64 / n as f64 * Complex::i()) * odd[k];
        x[k] = even[k] + t;
        x[k + n / 2] = even[k] - t;
    }
}

fn fft_loop(size: usize, loops: usize) {
    let mut rng = rand::thread_rng();

    let mut x: Vec<Complex<f64>>;

    for _i in 0..loops {
        x = (0..size).map(|_| rng_complex(&mut rng)).collect();
        fft(&mut x);
    }
}

fn main() {
    let args: Vec<String> = env::args().collect();

    if args.len() != 4 {
        println!("Usage: ./fft_rs <fft_size> <num_loops> <num_threads>");
        return;
    }

    let fft_size: usize = 1 << args[1].parse::<usize>().unwrap();
    let num_loops: usize = args[2].parse::<usize>().unwrap();
    let num_threads: usize = args[3].parse::<usize>().unwrap();

    let handles: Vec<_> = (0..num_threads)
        .map(|_| {
            let fs: usize = fft_size.clone();
            let nl: usize = num_loops.clone();
            thread::spawn(move || {
                fft_loop(fs, nl);
            })
        })
        .collect();

    for handle in handles {
        handle.join().unwrap();
    }
}
