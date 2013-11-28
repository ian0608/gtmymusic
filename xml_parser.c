//--------------------------------------
// Warren Shenk
//--------------------------------------

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <byteswap.h>
#include <ctype.h>
#include "gtmymusic.h"



//----------------------------------------------------------------------------
int get_playcount(char input[]) {
	char nameBuffer[258];
	memset(nameBuffer, 0, 258);
	char name[257];
	memset(name,0,257);
	char line[1024];
	char playCount[8];
	int pc;
	FILE *f;
	int count;

	f = fopen ("iTunes Music Library.xml","rw+");
	if (f == NULL) 
		printf("ERROR: File Didn't Open");

	while (fgets(line, sizeof(line), f)) {

		char *p = line;
		while(isspace(*p))
		p++;
		if(memcmp(p, "<key>Name</key><string>", 23) == 0) {
			// Get the name + tag into nameBuffer
			memcpy(nameBuffer, p + 23, 258);
			
			// Get rid of the tag, store result in name
			sscanf(nameBuffer, "%[^<]", name);

			// Get Character count for name			
			count = 0;
			while(name[++count] != '\0');

			// Compare input to found name
			if(memcmp(input, name, count) == 0) {
				// If match, find the playcount in the xml
				while (fgets(line, sizeof(line), f)) {
					// Extract playcount
					if(memcmp(p, "<key>Play Count</key><integer>", 30) == 0) {
						memcpy(playCount, p + 30, 8);
						sscanf(playCount, "%i<", &pc);	
						return pc;
					}
				}
			}
		}


	}
	fclose (f);
	return 0;
}
//----------------------------------------------------------------------------

