constexpr unsigned ARRAY_SIZE = (1u << 13u);  // 8 kb
char arr[ARRAY_SIZE];

constexpr int totalRound = 8;

int main(int argc, char **argv) {
    auto asso = ((unsigned) argv[0]);
    int sum = 0;
    int block_num = 1 << (asso + 2);
    int block_size = (2 * argc) / block_num;

    for (int i = 0; i < block_num; i += 2) {
        for (int j = 0; j < block_size; j++) {
            sum += arr[i * block_size + j];
        }
    }

    asm volatile(".word 0x0000000b");
    return sum;
}