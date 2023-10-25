use num_cpus;
use rand::Rng;
use std::thread;

fn main() {
    let num_threads = num_cpus::get();

    let handles: Vec<_> = (0..num_threads)
        .map(|_| {
            thread::spawn(|| {
                println!("Hello World!");
            })
        })
        .collect();

    for handle in handles {
        handle.join().unwrap();
    }
}
