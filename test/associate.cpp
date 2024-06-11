constexpr unsigned ARRAY_SIZE = (1u << 13u);
char arr[ARRAY_SIZE];


int main(int argc, char **argv) {
    auto cache_size = ((unsigned) argv[0]);
    auto n = ((unsigned) argv[1]);

    int sum = 0;
    int block_num = 1 << (n + 2);
    unsigned block_size = (2 * cache_size) / block_num;
    int round = 8;
    // unsigned block_size = 4096 / block_num;

    for (int j = 0; j < round; j++) {
        for (int i = 0; i < block_num; i += 2) {
            sum += arr[i * block_size];
        }
    }

//    for (int i = 0; i < block_size; i++) {
//        for (unsigned j = 0; j < 2 * cache_size; j += block_size * 2) {
//            sum += arr[j + i];
//        }
//    }

    asm volatile(".word 0x0000000b");
    return sum;
}