function test_control_flow() -> int {
    int result = 0;

    for (int i = 0; i < 5; i++) {
        result = result + i;
    }

    int j = 0;
    while (j < 5) {
        result = result + j;
        j = j + 1;
    }

    int x = 10;
    do {
        x = x - 1;
    } while (x > 5);

    return result + x;
}

function main() -> int {
    return test_control_flow();
}