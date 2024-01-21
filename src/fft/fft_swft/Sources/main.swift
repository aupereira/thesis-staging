import Foundation

public struct Complex {
    var real: Double
    var imag: Double

    init(_ real: Double, _ imag: Double) {
        self.real = real
        self.imag = imag
    }

    static func Imag() -> Complex {
        Complex(0, 1)
    }

    static func ToComplex(_ real: Double) -> Complex {
        Complex(real, 0)
    }

    func Add(_ b: Complex) -> Complex {
        Complex(real + b.real, imag + b.imag)
    }

    func Sub(_ b: Complex) -> Complex {
        Complex(real - b.real, imag - b.imag)
    }

    func Mul(_ b: Complex) -> Complex {
        Complex(real * b.real - imag * b.imag, real * b.imag + imag * b.real)
    }

    static func Exp(_ p: Complex) -> Complex {
        Complex(exp(p.real) * cos(p.imag), exp(p.real) * sin(p.imag))
    }
}

func randComplex() -> Complex {
    Complex(Double.random(in: 0 ... 1), Double.random(in: 0 ... 1))
}

func fft(_ x: inout [Complex]) {
    let n = x.count

    if n <= 1 {
        return
    }

    var even = [Complex]()
    var odd = [Complex]()

    for i in stride(from: 0, to: n, by: 2) {
        even.append(x[i])
        odd.append(x[i + 1])
    }

    fft(&even)
    fft(&odd)

    var t = Complex(0, 0)

    for k in 0 ..< n / 2 {
        t = Complex.Exp(Complex.ToComplex(-2.0 * Double.pi * Double(k) / Double(n)).Mul(Complex.Imag())).Mul(odd[k])
        x[k] = even[k].Add(t)
        x[k + n / 2] = even[k].Sub(t)
    }
}

func fftLoop(size: Int, loops: Int) async {
    let cmplx = Complex(0, 0)
    var x = [Complex](repeating: cmplx, count: size)

    for e in 0 ..< loops {
        for i in 0 ..< size {
            x[i] = randComplex()
        }
        fft(&x)
    }
}

func main() async {
    if CommandLine.arguments.count != 4 {
        print("Usage: ./fft_swft <FFT size> <num loops> <num threads>")
        return
    }

    let fftSize = 1 << Int(CommandLine.arguments[1])!
    let numLoops = Int(CommandLine.arguments[2])!
    let numThreads = Int(CommandLine.arguments[3])!

    var tasks = [Task<Void, Never>]()
    for _ in 0 ..< numThreads {
        let task = Task {
            await fftLoop(size: fftSize, loops: numLoops)
        }
        tasks.append(task)
    }

    for task in tasks {
        await task.value
    }
}

let semaphore = DispatchSemaphore(value: 0)

Task.detached {
    await main()
    semaphore.signal()
}

semaphore.wait()
