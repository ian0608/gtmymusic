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

int init_list_item_array(list_item_array **toInit);
list_item_array *get_list_items_current_dir();
void teardown_list_item_array(list_item_array *item_array);
list_item_array *diff_lists(list_item_array *authoritative, list_item_array *other);

