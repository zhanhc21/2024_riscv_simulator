//
// Created by zhanhc on 24-6-2.
//
constexpr unsigned ARRAY_SIZE = (1u << 12u);  // 8 kb
char arr[ARRAY_SIZE];

constexpr int totalRound = 64;

int main(int argc, char **argv) {
    auto testStep = ((unsigned) argv[0]);
    int sum = 0;

    for (int i = 0; i < totalRound; i++) {
        unsigned index = 0;
        // 相同访问次数
        for (unsigned j = 0; j < 512; ++j) {
            sum += arr[index];
            index = (index + testStep) % ARRAY_SIZE;
        }
    }

    asm volatile(".word 0x0000000b");

    return sum;
}