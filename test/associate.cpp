//constexpr unsigned ARRAY_SIZE = (1u << 13u);
//char arr[ARRAY_SIZE];
//
//
//int main(int argc, char **argv) {
//    auto cache_size = ((unsigned) argv[0]);
//    auto n = ((unsigned) argv[1]);
//
//    int sum = 0;
//    int block_num = 1 << (n + 2);
//    //unsigned block_size = (2 * cache_size) / block_num;
//    int round = 8;
//    unsigned block_size = ARRAY_SIZE / block_num;
//
//    for (int j = 0; j < round; j++) {
//        for (int i = 0; i < ARRAY_SIZE; i += 2*block_size) {
//            sum += arr[i];
//        }
//    }
//
//    asm volatile(".word 0x0000000b");
//    return sum;
//}
constexpr unsigned ARRAY_SIZE = (1u << 13u);  // 4096B
char arr[ARRAY_SIZE];

constexpr int totalTime = 8;
constexpr int totalRound = 512;

int main(int argc, char **argv) {
    auto cache_size = ((unsigned) argv[0]);
    auto n = ((unsigned) argv[1]);
    unsigned block_num = 1 << (n + 2);
    unsigned block_size = (cache_size * 2) / block_num;
    int sum = 0;

    for (int i = 0; i < block_size; i++)
        for (unsigned j = 0; j < block_num; j += 2)
            sum += arr[j * block_size];

    asm volatile(".word 0x0000000b");
    return sum;
}