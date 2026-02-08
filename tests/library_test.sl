function add_numbers(int a, int b) -> int {
    return a + b;
}

function multiply_numbers(int a, int b) -> int {
    return a * b;
}

function fibonacci(int n) -> int {
    if (n <= 1) {
        return n;
    }
    return fibonacci(n - 1) + fibonacci(n - 2);
}

const int VERSION = 100;