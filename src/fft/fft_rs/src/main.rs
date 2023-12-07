use num::complex::Complex;
use std::f64::consts::PI;
use rand::Rng;
use std::thread;
use std::env;

fn rng_complex() -> Complex<f64> {
    let mut rng = rand::thread_rng();
    let complex = num::complex::Complex::new(
        rng.gen_range(-1.0..1.0),
        rng.gen_range(-1.0..1.0)
    );
    return complex;
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
    let mut x: Vec<Complex<f64>>;
    
    for _i in 0..loops {
        x = (0..size).map(|_| rng_complex()).collect();
        fft(&mut x);
    }

    return;
}

fn main() {
    let args: Vec<String> = env::args().collect();

    if args.len() != 4 {
        println!("Usage: ./fft <fft_size> <num_loops> <num_threads>");
        return;
    }

    let fft_size: usize = 2usize.pow(args[1].parse::<u32>().unwrap());
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


    //Basic validation set for earlier development
    
    // let fft_size: u64 = 65536;
    // let num_loops: u64 = 10;
    // let num_threads: u64 = 12;

    // let a: Complex<f64> = Complex::new(1.0, 2.0);
    // let b: Complex<f64> = Complex::new(3.0, -4.0);
    // let c: Complex<f64> = Complex::new(5.0, 6.0);
    // let d: Complex<f64> = Complex::new(7.0, -8.0);
    
    // let mut x: Vec<Complex<f64>> = [a, b, c, d].to_vec();

    // fft(&mut x);

    // println!("{:?}", x);
}



// Might want to test this for speed.

// use num_cpus;
// use std::ops::{Add, Sub, Mul};

// struct Complex {
//     re: f64,
//     im: f64,
// }

// impl Add for Complex {
//     type Output = Complex;

//     fn add(self, other: Complex) -> Complex {
//         Complex {
//             re: self.re + other.re,
//             im: self.im + other.im,
//         }
//     }
// }

// impl Sub for Complex {
//     type Output = Complex;

//     fn sub(self, other: Complex) -> Complex {
//         Complex {
//             re: self.re - other.re,
//             im: self.im - other.im,
//         }
//     }
// }

// impl Mul for Complex {
//     type Output = Complex;

//     fn mul(self, other: Complex) -> Complex {
//         Complex {
//             re: self.re * other.re - self.im * other.im,
//             im: self.re * other.im + self.im * other.re,
//         }
//     }
// }

// impl Complex {
//     fn exp(self) -> Complex {
//         Complex {
//             re: self.re.exp() * self.im.cos(),
//             im: self.re.exp() * self.im.sin(),
//         }
//     }
// }

// fn rng_complex() -> Complex {
//     let mut rng = rand::thread_rng();
//     Complex {
//         re: rng.gen_range(-1.0..1.0),
//         im: rng.gen_range(-1.0..1.0),
//     }
// }