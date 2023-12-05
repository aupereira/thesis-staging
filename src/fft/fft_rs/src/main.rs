use num_cpus;
use num::complex::Complex;
use std::ops::{Add, Sub, Mul};
use std::f64::consts::PI;
use rand::Rng;
use std::thread;

pub struct Complex {
    pub re: T,
    pub im: T,
}

impl Mul for Complex {
    type Output = Complex;

    fn mul(self, other: Complex) -> Complex {
        Complex {
            re: self.re * other.re - self.im * other.im,
            im: self.re * other.im + self.im * other.re,
        }
    }
}

fn fft(x: &mut [f64]) -> Vec<Complex> {
    let N = x.len();

    let halfN = N / 2;
    
    if N <= 1 {
        return x;
    }

    let even = fft(x[0..N:2]);
    let odd = fft(x[1..N:2]);

    for k in 0..halfN {
        let t = Complex::from_polar(&1.0, (&-2.0 * PI * k as f64 / N as f64)) * odd[k];
        x[k] = even[k] + t;
        x[k + halfN] = odd[k] - t;
    }

    return x;
}

fn main() {
    // let num_threads = num_cpus::get();

    // let handles: Vec<_> = (0..num_threads)
    //     .map(|_| {
    //         thread::spawn(|| {
    //             println!("Hello World!");
    //         })
    //     })
    //     .collect();

    // for handle in handles {
    //     handle.join().unwrap();
    // }

    let mut x: Vec<f64> = (0..64).map(|_| rng.gen()).collect();
    println!(fft(&mut x))
}

//     if (ceil(log2(N)) != floor(log2(N))){
//     panic!("N is not a power of 2");
// }