def rand_complex
    Complex(rand, rand)
end

def fft(x)
    n = x.length
    
    return x if n <= 1

    even = Array.new(n / 2) { |i| x[2 * i] }
    odd = Array.new(n / 2) { |i| x[2 * i + 1] }

    fft(even)
    fft(odd)

    for k in 0..(n / 2 - 1)
        t = Math::E ** (-2 * Math::PI * k / n * Complex(0, 1)) * Complex(odd[k], 0)
        x[k] = even[k] + t
        x[k + n / 2] = even[k] - t
    end    
end

def fft_loop(size, loops)
    x = Array.new(size)

    loops.times do
        size.times do |i| 
            x[i] = rand_complex
        end
        fft(x)
    end
end

def main

    if ARGV.length != 3
        puts "Usage: ruby main.rb <FFT size> <num loops> <num threads>"
        exit 1
    end

    fft_size = 2**ARGV[0].to_i
    num_loops = ARGV[1].to_i
    num_threads = ARGV[2].to_i
    
    num_threads.times do |i|
        if (ENV['OS'] == 'Windows_NT')
            Process.spawn("ruby", "-e", "require './main'; fft_loop(#{fft_size}, #{num_loops})")
        else
            if fork.nil?
                fft_loop(fft_size, num_loops)
                exit 0
            end
        end
    end

    Process.waitall
end

if __FILE__ == $0
    main
end