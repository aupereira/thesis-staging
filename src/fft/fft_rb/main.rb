# frozen_string_literal: true

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

  (0...(n / 2)).each do |k|
    t = Math::E**(-2 * Math::PI * k / n * Complex(0, 1)) * Complex(odd[k], 0)
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
    puts 'Usage: ruby fft.rb <FFT size> <num loops> <num threads>'
    exit 1
  end

  fft_size = 1 << ARGV[0].to_i
  num_loops = ARGV[1].to_i
  num_threads = ARGV[2].to_i

  num_threads.times do |_i|
    if RUBY_ENGINE == 'jruby'
      Thread.new do
        fft_loop(fft_size, num_loops)
      end
    elsif RUBY_ENGINE == 'truffleruby'
      Process.spawn('truffleruby', '-e', "require './#{__FILE__}'; fft_loop(#{fft_size}, #{num_loops})")
    elsif RUBY_ENGINE == 'ruby'
      if ENV['OS'] == 'Windows_NT'
        Process.spawn('ruby', '-e', "require './#{__FILE__}'; fft_loop(#{fft_size}, #{num_loops})")
      elsif fork.nil?
        fft_loop(fft_size, num_loops)
        exit 0
      end
    end
  end

  Process.waitall
end

main if __FILE__ == $PROGRAM_NAME
