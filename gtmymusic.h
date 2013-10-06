#include <openssl/md5.h>
#include <dirent.h>

typedef struct {
	unsigned char hash[MD5_DIGEST_LENGTH];
	char filename[NAME_MAX + 1];
} list_item;

typedef struct {
	int count;
	list_item **items;
} list_item_array;
