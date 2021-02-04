#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <array>

#include "../syscall.h"

struct UIntBuffer {
    char data[4];
    unsigned int to_uint() {
        uint32_t v = 0;
        for(int i = 0; i < 4; ++i) {
            v |= static_cast<unsigned char>(data[i]) << (i * 8);
        }
        return v;
    }
};

bool read_image_size(FILE* fp, unsigned int& width, unsigned int& height) {
    UIntBuffer width_buf;
    UIntBuffer height_buf;
    if(fread(width_buf.data, sizeof(char), 4, fp) != 4) {
        return false;
    }

    if(fread(height_buf.data, sizeof(char), 4, fp) != 4) {
        return false;
    }
    
    width = width_buf.to_uint();
    height = height_buf.to_uint();

    return true;
}

extern "C" void main(int argc, char** argv) {
    bool is_command_clear = false;

    int width, height;
    const char* path;

    if(argc == 2) {
        if(strcmp(argv[1], "-c") == 0) {
            is_command_clear = true;
        } else {
            path = argv[1];
        }
    } else {
        printf(R"(usage:
    setbg -c
        : clear background image
    setbg /path/to/image
        : set the window background with an image
)");
        exit(0);
    }

    if(is_command_clear) {
        auto result = SyscallClearDesktopBgImage();
        if(result.error != 0) {
            printf("error caused while processing image: %d", result.error);
            exit(1);
        }
    } else {
        FILE* fp = fopen(path, "r");
        if (fp == nullptr) {
            printf("failed to open: %s\n", path);
            exit(1);
        }

        char* buf = new char [640 * 480 * 3];

        unsigned int width, height;
        if(!read_image_size(fp, width, height)) {
            printf("failed to load image size\n");
            exit(1);
        }

        if (fread(buf, 3 * sizeof(char), width * height, fp) != width * height) {
            printf("failed to load image content\n");
            exit(1);
        }

        auto result = SyscallSetDesktopBgImage(width, height, buf);
        delete[] buf;

        if(result.error != 0) {
            printf("error caused while processing image: %d", result.error);
            exit(1);
        }

    }

    exit(0);
}
