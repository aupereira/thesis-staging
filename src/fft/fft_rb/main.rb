# frozen_string_literal: true

def fft(x)
  n = x.length

  return x if n == 2

  even = Array.new(n / 2)
  odd = Array.new(n / 2)

  (0...(n / 2)).step(2) do |i|
    even[i] = x[2 * i]
    even[i + 1] = x[2 * i + 1]
    odd[i] = x[2 * i + 2]
    odd[i + 1] = x[2 * i + 3]
  end

  fft(even)
  fft(odd)

  (0...(n / 2)).step(2) do |k|
    imag = -2 * Math::PI * k / n
    real = Math.cos(imag)
    imag = Math.sin(imag)

    t_real = real * odd[k] - imag * odd[k + 1]
    t_imag = real * odd[k + 1] + imag * odd[k]

    x[k] = even[k] + t_real
    x[k + 1] = even[k + 1] + t_imag

    x[k + n / 2] = even[k] - t_real
    x[k + n / 2 + 1] = even[k + 1] - t_imag
  end
end

def fft_loop(size, loops)
  x = Array.new(2 * size)

  loops.times do
    (2*size).times do |i|
      x[i] = rand()
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
