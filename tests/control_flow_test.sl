function main() -> int {
    int result = 0;
    if (true) {
        result = 10;
    } else {
        result = 20;
    }

    for (int i = 0; i < 3; i++) {
        result += i;
    }

    return result;
}