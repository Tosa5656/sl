function add(int a, int b) -> int {
    return a + b;
}

function multiply(int a, int b) -> int {
    return a * b;
}

function power(int base, int exp) -> int {
    if (exp == 0) {
        return 1;
    } else {
        return base * power(base, exp - 1);
    }
}

function fibonacci(int n) -> int {
    if (n <= 1) {
        return n;
    } else {
        return fibonacci(n - 1) + fibonacci(n - 2);
    }
}

function factorial(int n) -> int {
    if (n <= 1) {
        return 1;
    } else {
        return n * factorial(n - 1);
    }
}

function complex_calculation(int x, int y, int z) -> int {
    int a = add(x, y);
    int b = multiply(a, z);
    int c = power(b, 2);
    return c;
}

function test_functions() -> int {
    int sum = add(5, 10);
    int prod = multiply(3, 4);
    int pow2 = power(2, 3);
    int fib5 = fibonacci(5);
    int fact5 = factorial(5);
    int complex = complex_calculation(2, 3, 4);

    // Return sum of results as verification
    return sum + prod + pow2 + fib5 + fact5 + complex;
}

function main() -> int {
    int result = test_functions();
    // Expected: 15 + 12 + 8 + 5 + 120 + 400 = 560
    return result;
}