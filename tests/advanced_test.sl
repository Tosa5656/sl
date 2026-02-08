const int MAX_VALUE = 100;

function fibonacci(int n) -> int {
    if (n <= 1) {
        return n;
    }
    return fibonacci(n - 1) + fibonacci(n - 2);
}

function test_switch(int value) -> int {
    int result = 0;
    switch (value) {
        case 1:
            result = 10;
            break;
        case 2:
            result = 20;
            break;
        default:
            result = 0;
    }
    return result;
}

function test_loops() -> int {
    int sum = 0;

    // Test for loop
    for (int i = 0; i < 5; i++) {
        sum += i;
    }

    // Test while loop
    int j = 0;
    while (j < 3) {
        sum += 10;
        j++;
    }

    // Test do-while loop
    int k = 0;
    do {
        sum += 5;
        k++;
    } while (k < 2);

    return sum;
}

function main() -> int {
    int fib = fibonacci(6);
    int switch_result = test_switch(2);
    int loop_result = test_loops();
    return fib + switch_result + loop_result;
}