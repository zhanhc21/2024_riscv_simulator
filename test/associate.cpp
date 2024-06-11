constexpr unsigned ARRAY_SIZE = (1u << 20u);
char arr[ARRAY_SIZE];


int main(int argc, char **argv) {
    auto cache_size = ((unsigned) argv[0]);
    auto n = ((unsigned) argv[1]);

    int sum = 0;
    int block_num = 1 << (n + 2);
    // unsigned block_size = (2 * cache_size) / block_num;
    unsigned block_size = 4096 / block_num;

    for (int i = 0; i < block_num; i += 2) {
        for (int j = 0; j < block_size; j++) {
            sum += arr[i * block_size + j];
        }
    }

    asm volatile(".word 0x0000000b");
    return sum;
}