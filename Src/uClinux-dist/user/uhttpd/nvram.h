#define MAX_NVRAM_BLOCK_SIZE 0x2000

#define NVRAM_CONFIG 0
#define NVRAM_CONFIG_MTD "/dev/mtdblock2"
#define NVRAM_TEMPLATE 1
#define NVRAM_TEMPLATE_MTD "/dev/mtdblock3"

void reloadNVRAMs();
const char* readNVRAM(int nvram, int start, int* size);
int eraseNVRAM(int nvram);
int writeNVRAM(int nvram, const char* data, int start, int size);
int getNVRAMUsedBytes(int nvram);
int getNVRAMSize(int nvram);

int nvram_get_variable(const char* variable_name, char* content, int content_len);
int nvram_set_variable(const char* variable_name, const char* content);
