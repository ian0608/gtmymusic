#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "gtmymusic.h"



void DieWithErr(char *errorMessage){
    printf("%s\n", errorMessage);
    exit(EXIT_FAILURE);
}

void Err(char *errorMessage) {
    printf("%s\n", errorMessage);
}

/*
Requires linking with -lcrypto
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

int delete_index_from_array(list_item_array **toDeleteFrom, int index) {
	if (index < 0 || index >= (*toDeleteFrom)->count) 
		return -1;
	if (index != (*toDeleteFrom)->count-1) {
		memcpy((*toDeleteFrom)->items[index], (*toDeleteFrom)->items[(*toDeleteFrom)->count-1], sizeof(list_item));
	}
	free((*toDeleteFrom)->items[(*toDeleteFrom)->count-1]);
	(*toDeleteFrom)->count --;
	(*toDeleteFrom)->items = realloc((*toDeleteFrom)->items,(*toDeleteFrom)->count*sizeof(list_item));
	if((*toDeleteFrom)->items == NULL)
		return -1;
	return 0;

}

int sort_descending_playcount(list_item_array **toSort) {
	list_item *temp = malloc(sizeof(list_item));
	int count = (*toSort)->count;
	int i;
	int j;
	
	// BUBBLESORT
	for (i = 0 ; i < (count - 1); i++) {
		for (j = 0 ; j < (count - i - 1); j++) {
			if ((*toSort)->items[j]->playcount < (*toSort)->items[j+1]->playcount) 
			{
				memcpy(temp, (*toSort)->items[j], sizeof(list_item)); 
				memcpy((*toSort)->items[j], (*toSort)->items[j+1], sizeof(list_item)); 
				memcpy((*toSort)->items[j+1], temp, sizeof(list_item)); 
		        }
		}
	}
	free(temp);
	return 0;
}

/*
This function returns a pointer to a list_item_array representing the mp3 files in the current directory. Each individual list_item contains the filename and MD5 hash of the file contents. See main for an example.
*/

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
			int32_t filesize;
			FILE *file = fopen(entry->d_name, "rb");
			if (file == NULL)
			{
				return NULL;
			}

			/* COMPUTE FILE SIZE */
		    	if (fseek(file, 0, SEEK_END) == 0) {
				filesize = ftell(file);
		       		if (filesize == -1) 
			    		Err("ftell() failed to SEEK_END");
		    	}
			else {
				Err("fseek() failed failed to SEEK_END");
			}
		    	if (fseek(file, 0, SEEK_SET) != 0) {
				Err("fseek() failed to SEEK_SET");
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

			item_array->items[item_array->count-1]->playcount = get_playcount(entry->d_name);
			item_array->items[item_array->count-1]->filesize = filesize;
			memcpy(item_array->items[item_array->count-1]->hash, hash, MD5_DIGEST_LENGTH);
			memcpy(item_array->items[item_array->count-1]->filename, entry->d_name, strlen(entry->d_name)+1);
			//printf("Filename: %s PlayCount: %i\n", entry->d_name, item_array->items[item_array->count-1]->playcount);

			
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

/*
This function returns a pointer to a list_item_array containing the hash/filename pairs contained in "authoritative" but not in "other." See main for an example.
*/

list_item_array *diff_lists(list_item_array *authoritative, list_item_array *other)
{
	list_item_array *diff_list;
	if(init_list_item_array(&diff_list) < 0)
		return NULL;

	int i;
	for (i=0; i < authoritative->count; i++)
	{
		int found = 0;
		int j;
		for (j=0; j < other->count; j++)
		{
			if(memcmp(authoritative->items[i]->hash, other->items[j]->hash, MD5_DIGEST_LENGTH) == 0)
			{
				found = 1;
			}
		}
		if (found == 0)
		{
			if(incr_size_list_item_array(&diff_list) < 0)
				return NULL;

			memcpy(diff_list->items[diff_list->count-1]->hash, authoritative->items[i]->hash, MD5_DIGEST_LENGTH);
			memcpy(diff_list->items[diff_list->count-1]->filename, authoritative->items[i]->filename, strlen(authoritative->items[i]->filename)+1);
		}
	}
	return diff_list;
}

/*
int main(int argc, char *argv[])
{
	list_item_array *myList = get_list_items_current_dir();
	if (myList == NULL)
	{
		return -1;
	}

	printf("list creation test\n");
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

	printf("diff test\n");
	list_item_array *myList2 = get_list_items_current_dir();
	if (myList2 == NULL)
	{
		return -1;
	}
	if(incr_size_list_item_array(&myList2) < 0)
		return -1;
	char testFilename[] = "test.mp3";
	char testHash[] = "abcdefg";
	memcpy(myList2->items[myList2->count-1]->hash, testHash, 7);
	memcpy(myList2->items[myList2->count-1]->filename, testFilename, strlen(testFilename)+1);
	list_item_array *diff = diff_lists(myList2, myList);
	if (diff == NULL)
	{
		return -1;
	}
	i=0;
	while (i < diff->count)	//for each list_item
	{
		printf("%s\n", diff->items[i]->filename);	//print the filename
		int j;
		for (j=0; j < MD5_DIGEST_LENGTH; j++)				//and print the hash
			printf("%02x", diff->items[i]->hash[j]);
		printf("\n");
		i++;
	}


	teardown_list_item_array(myList);
	teardown_list_item_array(myList2);
	teardown_list_item_array(diff);

	return 0;
}
*/
