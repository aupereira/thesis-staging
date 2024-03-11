import math
import multiprocessing
import random
import sys


def fft(x):
    n = len(x)

    if n == 2:
        return x

    even = [0 for _ in range(n // 2)]
    odd = [0 for _ in range(n // 2)]

    for i in range(0, n // 2, 2):
        even[i] = x[2 * i]
        even[i + 1] = x[2 * i + 1]
        odd[i] = x[2 * i + 2]
        odd[i + 1] = x[2 * i + 3]

    even = fft(even)
    odd = fft(odd)

    for k in range(0, n // 2, 2):
        imag = -2 * math.pi * k / n
        real = math.cos(imag)
        imag = math.sin(imag)

        t_real = real * odd[k] - imag * odd[k + 1]
        t_imag = real * odd[k + 1] + imag * odd[k]

        x[k] = even[k] + t_real
        x[k + 1] = even[k + 1] + t_imag

        x[k + n // 2] = even[k] - t_real
        x[k + n // 2 + 1] = even[k + 1] - t_imag

    return x


def fft_loop(size, loops):
    for _ in range(loops):
        x = [random.random() for _ in range(2 * size)]
        fft(x)


def main():
    if len(sys.argv) != 4:
        print("Usage: py fft.py <FFT size> <num loops> <num threads>")
        return

    fft_size = 1 << int(sys.argv[1])
    num_loops = int(sys.argv[2])
    num_threads = int(sys.argv[3])

    processes = []

    for i in range(num_threads):
        processes.append(multiprocessing.Process(
            target=fft_loop, args=(fft_size, num_loops)
        ))
        processes[i].start()

    for process in processes:
        process.join()


if __name__ == '__main__':
    main()
