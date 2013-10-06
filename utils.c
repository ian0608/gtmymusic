#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "gtmymusic.h"


/*
Requires linking with -lcrypto

This function returns a pointer to a list_item_array representing the mp3 files in the current directory. Each individual list_item contains the filename and MD5 hash of the file contents. See main for an example.
*/

int init_list_item_array(list_item_array **toInit)
{
	*toInit = malloc(sizeof(list_item_array));
	if (*toInit == NULL)
	{
		return -1;
	}

	(*toInit)->count = 0;

	(*toInit)->items = malloc(sizeof(list_item *));
	if((*toInit)->items == NULL)
	{
		return -1;
	}
	return 0;
}

int incr_size_list_item_array(list_item_array **toIncr)
{
	(*toIncr)->count++;
	(*toIncr)->items = realloc((*toIncr)->items, (*toIncr)->count*sizeof(list_item *));
	if((*toIncr)->items == NULL)
	{
		return -1;
	}

	(*toIncr)->items[(*toIncr)->count -1] = malloc(sizeof(list_item));
	if((*toIncr)->items[(*toIncr)->count -1] == NULL)
	{
		return -1;
	}
	return 0;
}

list_item_array *get_list_items_current_dir()
{
	DIR *directory;
	struct dirent *entry;

	if ((directory = opendir(".")) == NULL)
	{
		printf("can't open current dir\n");
		return NULL;
	}

	entry = readdir(directory);

	list_item_array *item_array;
	if(init_list_item_array(&item_array) < 0)
		return NULL;

	while(entry != NULL)
	{
		if (strcmp((entry->d_name + strlen(entry->d_name) - 4), ".mp3") == 0)
		{
			//printf("%s\n", entry->d_name);
			
			if(incr_size_list_item_array(&item_array) < 0)
				return NULL;
			
			//hash & filename
			unsigned char hash[MD5_DIGEST_LENGTH];
			FILE *file = fopen(entry->d_name, "rb");
			if (file == NULL)
			{
				return NULL;
			}
			MD5_CTX context;
			int read;
			unsigned char buffer[1000];
			MD5_Init(&context);
			while ((read = fread(buffer, 1, 1000, file)) != 0)
			{
				MD5_Update(&context, buffer, read);
			}
			fclose(file);
			MD5_Final(hash, &context);			
			
			memcpy(item_array->items[item_array->count-1]->hash, hash, MD5_DIGEST_LENGTH);
			memcpy(item_array->items[item_array->count-1]->filename, entry->d_name, strlen(entry->d_name)+1);

			
		}
		entry = readdir(directory);
	}

	closedir(directory);

	if (item_array->count == 0)
	{
		return NULL;
	}
	else
	{
		return item_array;
	}
}

void teardown_list_item_array(list_item_array *item_array)
{
	int i=0;
	while (i < item_array->count)	//for each list_item
	{
		free(item_array->items[i]);
		i++;
	}
	free(item_array->items);
	free(item_array);
}

int main(int argc, char *argv[])
{
	list_item_array *myList = get_list_items_current_dir();
	if (myList == NULL)
	{
		return -1;
	}

	int i=0;
	while (i < myList->count)	//for each list_item
	{
		printf("%s\n", myList->items[i]->filename);	//print the filename
		int j;
		for (j=0; j < MD5_DIGEST_LENGTH; j++)				//and print the hash
			printf("%02x", myList->items[i]->hash[j]);
		printf("\n");
		i++;
	}

	teardown_list_item_array(myList);

	return 0;
}
