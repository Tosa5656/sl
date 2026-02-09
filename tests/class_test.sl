class SimpleClass {
    SimpleClass() {
        // Default constructor
    }
}

class ClassWithParams {
    ClassWithParams(int value) {
        // Constructor with parameter
    }
}

function test_simple_class() -> int {
    return 0;
}

function test_class_with_params() -> int {
    return 0;
}

function main() -> int {
    int result1 = test_simple_class();
    int result2 = test_class_with_params();

    return result1 + result2;
}