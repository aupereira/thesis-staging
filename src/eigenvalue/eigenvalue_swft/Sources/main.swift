import Foundation

func printVec(_ v: [Double]) {
    let n = v.count
    
    print()
    for i in 0..<n {
        print(v[i], terminator: " ")
    }
    print()
}

func printMatrix(_ A: [[Double]]) {
    let n = A.count
    
    print()
    for i in 0..<n {
        for j in 0..<n {
            print(A[i][j], terminator: " ")
        }
        print()
    }
}

func sgn(_ x: Double) -> Double {
    if x >= 0 {
        return 1
    } else {
        return -1
    }
}

func randomSymmetric(_ A: inout [[Double]]) {
    let n = A.count

    for i in 0..<n {
        A[i][i] = Double.random(in: 0...1)
        for j in i+1..<n {
            A[i][j] = Double.random(in: 0...1)
            A[j][i] = A[i][j]
        }
    }
}

func norm2(_ len: Int, _ v: [Double]) -> Double {
    var norm: Double = 0
    for i in 0..<len {
        norm += v[i] * v[i]
    }
    return sqrt(norm)
}

func vDivS(_ len: Int, _ v: inout [Double], _ s: Double) {
    for i in 0..<len {
        v[i] /= s
    }
}

func S2OPAI(_ len: Int, _ v: [Double], _ A: inout [[Double]]) {
    for i in 0..<len {
        for j in 0..<len {
            A[i][j] = -2.0 * v[i] * v[j]
        }
    }

    for i in 0..<len {
        A[i][i] += 1.0
    }
}

func matrixMultiply(
    _ len: Int,
    _ A: [[Double]],
    _ B: [[Double]],
    _ C: inout [[Double]],
    _ BTMP: inout [[Double]]
) {
    var sum: Double

    for i in 0..<len {
        for j in 0..<len {
            BTMP[i][j] = B[j][i]
        }
    }
    
    for i in 0..<len {
        for j in 0..<len {
            sum = 0
            for k in 0..<len {
                sum += A[i][k] * BTMP[j][k]
            }
            C[i][j] = sum
        }
    }
}

func tridiagonalize(
    _ n: Int,
    _ A: inout [[Double]],
    _ P: inout [[Double]],
    _ TEMP: inout [[Double]],
    _ TPOSE: inout [[Double]],
    _ v: inout [Double]
) {
    var alpha: Double
    var r: Double

    for k in 0..<n-2 {
        for j in 0..<k+1 {
            v[j] = 0
        }
        for j in k+1..<n {
            v[j] = A[j][k]
        }

        alpha = -sgn(v[k+1]) * norm2(n, v)
        r = 2 * sqrt(0.5 * ((alpha * alpha) - (alpha * v[k+1])))

        v[k+1] = (v[k+1] - alpha) / r
        for j in k+2..<n {
            v[j] /= r
        }

        S2OPAI(n, v, &P)
        matrixMultiply(n, P, A, &TEMP, &TPOSE)
        matrixMultiply(n, TEMP, P, &A, &TPOSE)
    }
}

func householder(
    _ len: Int,
    _ A: inout [[Double]],
    _ A1: inout [[Double]],
    _ Q: inout [[Double]],
    _ H: inout [[Double]],
    _ TEMP: inout [[Double]],
    _ TPOSE: inout [[Double]],
    _ v: inout [Double]
) {
    for i in 0..<len {
        for j in 0..<len {
            A1[i][j] = A[i][j]
        }
    }

    var SWAP: [[Double]]

    let s = A1[len - 1][len - 1]
    let t = A1[len - 2][len - 2]
    let x = A1[len - 2][len - 1]
    let d = (t - s) / 2
    let shift = s - x * x / (d + sgn(d) * sqrt(d * d + x * x))

    for i in 0..<len {
        A1[i][i] -= shift
    }

    for k in 0..<len-1 {
        for j in 0..<k {
            v[j] = 0
        }
        for j in k..<len {
            v[j] = A1[j][k]
        }

        v[k] += norm2(len, v) * sgn(v[k + 1])
        vDivS(len, &v, norm2(len, v))

        if k == 0 {
            S2OPAI(len, v, &Q)
            matrixMultiply(len, Q, A1, &TEMP, &TPOSE)
            SWAP = A1
            A1 = TEMP
            TEMP = SWAP
        }
        else {
            S2OPAI(len, v, &H)
            matrixMultiply(len, H, A1, &TEMP, &TPOSE)
            SWAP = A1
            A1 = TEMP
            TEMP = SWAP
            matrixMultiply(len, Q, H, &TEMP, &TPOSE)
            SWAP = Q
            Q = TEMP
            TEMP = SWAP
        }
    }

    matrixMultiply(len, A1, Q, &A, &TPOSE)

    for i in 0..<len {
        A[i][i] += shift
    }
}

func eigQR(
    _ A: inout [[Double]],
    _ eigVals: inout [Double],
    _ T1: inout [[Double]],
    _ T2: inout [[Double]],
    _ T3: inout [[Double]],
    _ T4: inout [[Double]],
    _ T5: inout [[Double]],
    _ v: inout [Double]
) {
    var ind = A.count

    var currentEigval: Double
    var prevEigval: Double = Double.greatestFiniteMagnitude

    while ind > 1 {
        householder(ind, &A, &T1, &T2, &T3, &T4, &T5, &v)

        currentEigval = A[ind - 1][ind - 1]

        if abs(currentEigval - prevEigval) < 1e-5 {
            eigVals[ind - 1] = currentEigval

            ind -= 1
            
            prevEigval = A[ind - 1][ind - 1]
        }
        else {
            prevEigval = currentEigval
        }
    }

    eigVals[0] = A[0][0]
}

func eigenLoop(size: Int, loops: Int) async -> Void {
    var A: [[Double]] = Array(repeating: Array(repeating: 0.0, count: size), count: size)

    var T1: [[Double]] = Array(repeating: Array(repeating: 0.0, count: size), count: size)
    var T2: [[Double]] = Array(repeating: Array(repeating: 0.0, count: size), count: size)
    var T3: [[Double]] = Array(repeating: Array(repeating: 0.0, count: size), count: size)
    var T4: [[Double]] = Array(repeating: Array(repeating: 0.0, count: size), count: size)
    var T5: [[Double]] = Array(repeating: Array(repeating: 0.0, count: size), count: size)
    var v: [Double] = Array(repeating: 0.0, count: size)

    var eigVals: [Double] = Array(repeating: 0.0, count: size)

    for _ in 0..<loops {
        randomSymmetric(&A)
        tridiagonalize(size, &A, &T1, &T2, &T3, &v)
        eigQR(&A, &eigVals, &T1, &T2, &T3, &T4, &T5, &v)
    }
}

func main() async {
    if CommandLine.arguments.count != 4 {
        print("Usage: ./fft_swft <FFT size> <num loops> <num threads>")
        return
    }

    let mSize = Int(CommandLine.arguments[1])!
    let numLoops = Int(CommandLine.arguments[2])!
    let numThreads = Int(CommandLine.arguments[3])!

    var tasks = [Task<Void, Never>]()
    for _ in 0..<numThreads {
        let task = Task {
            await eigenLoop(size: mSize, loops: numLoops)
        }
        tasks.append(task)
    }

    for task in tasks {
        await task.value
    }

    // let mSize = 4

    // var A = [
    //     [4.0, 1.0, -2.0, 2.0],
    //     [1.0, 2.0, 0.0, 1.0],
    //     [-2.0, 0.0, 3.0, -2.0],
    //     [2.0, 1.0, -2.0, -1.0]
    // ]

    // var T1: [[Double]] = Array(repeating: Array(repeating: 0.0, count: mSize), count: mSize)
    // var T2: [[Double]] = Array(repeating: Array(repeating: 0.0, count: mSize), count: mSize)
    // var T3: [[Double]] = Array(repeating: Array(repeating: 0.0, count: mSize), count: mSize)
    // var T4: [[Double]] = Array(repeating: Array(repeating: 0.0, count: mSize), count: mSize)
    // var T5: [[Double]] = Array(repeating: Array(repeating: 0.0, count: mSize), count: mSize)
    // var v: [Double] = Array(repeating: 0.0, count: mSize)

    // var eigVals: [Double] = Array(repeating: 0.0, count: mSize)

    // tridiagonalize(mSize, &A, &T1, &T2, &T3, &v)
    // eigQR(&A, &eigVals, &T1, &T2, &T3, &T4, &T5, &v)

    // printVec(eigVals)
}

let semaphore = DispatchSemaphore(value: 0)

Task.detached {
    await main()
    semaphore.signal()
}

semaphore.wait()