constexpr unsigned ARRAY_SIZE = (1u << 13u);  // 8 kb
char arr[ARRAY_SIZE];

constexpr int totalRound = 2;

int main(int argc, char **argv) {
    auto testStep = ((unsigned) argv[0]);
    int sum = 0;
    unsigned index = 0;

    for (int i = 0; i < totalRound; i++) {
        // 相同访问次数
        for (unsigned j = 0; j < 2048; j++) {
            sum += arr[index];
            index = (index + testStep) % ARRAY_SIZE;
        }
    }

    asm volatile(".word 0x0000000b");
    return sum;
}