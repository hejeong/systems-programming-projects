#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <openssl/sha.h>

char * createHashcode(char * fileStream, size_t length, char* buffer){
	int i = 0;
	char shaConverted[2];
	unsigned char hash[SHA256_DIGEST_LENGTH];
	
	SHA256(fileStream, length, hash);
	strcpy(buffer, "");
	for(i = 0; i < SHA256_DIGEST_LENGTH; i++){
        sprintf(shaConverted, "%02x", hash[i]);
		strcat(buffer, shaConverted);
	}	
	return buffer;
}

void addCommand(char* filePath, char* manifestPath){
	// get linked list of manifest file
	struct node * head;
	head = createManifestList(manifestPath, head);

	size_t length;
	char hashcode[2*SHA256_DIGEST_LENGTH];
	int fileBytes = getFileSizeInBytes(filePath);
	int fileDesc = open(filePath, O_RDONLY);
	if(fileDesc == -1){
 	  perror("Error opening file");
     	  return;
	}
	char* stream = malloc((fileBytes+1)*sizeof(char));
	read(fileDesc, stream, fileBytes);
	stream[fileBytes] = '\0';
	length = strlen(stream);
	strcpy(hashcode, createHashcode(stream, length, hashcode));
	free(stream);
	int closeStatus = close(fileDesc);
	head = addFileToList(1,filePath,hashcode, head);
	writeToManifest(manifestPath, head);
}
