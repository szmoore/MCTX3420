#include <openssl/sha.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define SALT_LENGTH 20

int ReadBytes(char *str, unsigned char *buffer, size_t buffer_length) {
	unsigned i, val;
	if (strlen(str) != buffer_length * 2)
		return 0;
	for (i = 0; i < buffer_length; i++) {
		sscanf(str + i*2, "%2x", &val);
		buffer[i] = (unsigned char) val;
	}
	return 1;
}

unsigned char *HashPass(const char *pass, unsigned char salt[SALT_LENGTH]) {
	unsigned char *buffer, *result;
	size_t pass_length = strlen(pass);
	size_t buffer_length = pass_length + SALT_LENGTH;
	buffer = malloc(buffer_length * sizeof(unsigned char));
	if (!buffer)
		return NULL;
		
	memcpy(buffer, pass, pass_length);
	memcpy(buffer + pass_length, salt, SALT_LENGTH);
	result = SHA1(buffer, buffer_length, NULL);
	free(buffer);
	
	return result;
}

int WriteUserPass(FILE *fp, const char *user, const char *pass) {
	unsigned char salt[SALT_LENGTH], *sha1;
	size_t i;
	
	FILE *fpr = fopen("/dev/urandom", "r");
	if (!fpr)
		return 0;
	fread(salt, sizeof(unsigned char), SALT_LENGTH, fpr);
	fclose(fpr);
	
	if (!fp)
		return 0;
	
	sha1 = HashPass(pass, salt);
	if (!sha1)
		return 0;
	
	fprintf(fp, "%s:", user);
	for (i = 0; i < SALT_LENGTH; i++) {
		fprintf(fp, "%02x", salt[i]);
	}
	fprintf(fp, "$");
	for (i = 0; i < 20; i++) {
		fprintf(fp, "%02x", sha1[i]);
	}
	fprintf(fp, "\n");
	
	return 1;
}

int CheckUserPass(const char *passfile, const char *cuser, const char *cpass) {
	FILE *fp = fopen(passfile, "r");
	char buffer[BUFSIZ];
	int ret = 0;
	if (!fp)
		return 0;
		
	while (fgets(buffer, BUFSIZ, fp)) {
		char *user, *salt, *hash, *ptr;
		
		user = buffer;
		ptr = strchr(buffer, ':');
		if (ptr) {
			*ptr++ = 0;
			salt = ptr;
			ptr = strchr(ptr, '$');
			if (ptr) {
				*ptr++ = 0;
				hash = ptr;
				ptr = strchr(ptr, '\n');
				if (ptr)
					*ptr = 0;
				
				if (strlen(hash) != 20 * 2) {
					printf("Invalid SHA-1 hash: %s\n", hash);
					continue;
				} else if (strlen(salt) != SALT_LENGTH * 2) {
					printf("Invalid salt length: %s\n", salt);
					continue;
				} else if (strcmp(user, cuser)) {
					continue;
				}
				
				unsigned char saltbytes[SALT_LENGTH], hashbytes[20];
				ReadBytes(salt, saltbytes, SALT_LENGTH);
				ReadBytes(hash, hashbytes, 20);
				if (!memcmp(HashPass(cpass, saltbytes), hashbytes, 20)) {
					printf("Matched with user: %s\n", cuser);
					ret = 1;
					break;
				}
				
			}
		}
	}
	
	fclose(fp);
	return ret;
}

int main(int argc, char *argv[]) {
	if (argc != 4) {
		printf("Usage: %s user pass fname\n", argv[0]);
		return 1;
	}
	
	FILE *fp = fopen(argv[3], "w");

	if (!WriteUserPass(fp, argv[1], argv[2])) {
		fprintf(stderr, "Failed to hash: %s:%s\n", argv[1], argv[2]);
		return 1;
	}
	fclose(fp);
	
	CheckUserPass(argv[3], argv[1], argv[2]);
	return 0;
}
