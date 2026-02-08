function add(int a, int b) -> int {
    return a + b;
}

function factorial(int n) -> int {
    if (n <= 1) {
        return 1;
    } else {
        return n * factorial(n - 1);
    }
}

function main() -> int {
    int sum = add(5, 10);
    int fact = factorial(5);
    return sum + fact;
}