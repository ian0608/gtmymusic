#include <openssl/md5.h>
#include <dirent.h>

typedef struct {
	unsigned char hash[MD5_DIGEST_LENGTH];
	char filename[NAME_MAX + 1];
	int64_t filesize;
	int playcount;
} list_item;

typedef struct {
	int32_t count;
	list_item **items;
} list_item_array;

void DieWithErr(char *errorMessage);
void Err(char *errorMessage);
int init_list_item_array(list_item_array **toInit);
int incr_size_list_item_array(list_item_array **toIncr);
int delete_index_from_array(list_item_array **toDeleteFrom, int index);
int sort_descending_playcount(list_item_array **toSort);
list_item_array *get_list_items_current_dir();
void teardown_list_item_array(list_item_array *item_array);
list_item_array *diff_lists(list_item_array *authoritative, list_item_array *other);

int get_playcount(char input[]);
