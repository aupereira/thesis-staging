public class Complex {
    private double real;
    private double imaginary;

    public Complex(double real, double imaginary) {
        this.real = real;
        this.imaginary = imaginary;
    }
    
    public static Complex fromReal(double real) {
        return new Complex(real, 0.0);
    }

    public static Complex randComplex() {
        return new Complex(Math.random(), Math.random());
    }

    public static Complex imag() {
        return new Complex(0.0, 1.0);
    }

    public Complex add(Complex b) {
        return new Complex(this.real + b.real, this.imaginary + b.imaginary);
    }

    public Complex sub(Complex b) {
        return new Complex(this.real - b.real, this.imaginary - b.imaginary);
    }

    public Complex mul(Complex b) {
        return new Complex(this.real * b.real - this.imaginary * b.imaginary, this.real * b.imaginary + this.imaginary * b.real);
    }

    public static Complex exp(Complex b) {
        return new Complex(Math.exp(b.real) * Math.cos(b.imaginary), Math.exp(b.real) * Math.sin(b.imaginary));
    }

    @Override
    public String toString() {
        return "{" + this.real + " + " + this.imaginary + "i}";
    }
}