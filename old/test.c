#include <stdio.h>

void write(FILE * file) {
    for (int i = 0; i < 256; ++i)
        fwrite((char*)&i, sizeof(i), 1, file);
}

void read1(FILE * file) {
    for (int i = 0; i < 256; ++i) {
        int x;
        fread((char*)&x, sizeof(x), 1, file);
    }
}

void read2(FILE * file) {
    for (int j = 0; j < 10; ++j)
        for (int i = 0; i < 1000; i+=129) {
            int x;
            fseek(file, i, SEEK_SET);
            fread((char*)&x, sizeof(x), 1, file);
        }
}

int main() {
    FILE * file = fopen("test.bin", "w+b");
    /*
    for (int i = 0; i < 1000; ++i)
        write(file);
        */
    for (int i = 0; i < 10000; ++i)
        read2(file);
    fclose(file);
}
