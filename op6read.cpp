/*
 * File: op6read.cpp
 *
 * KORG opsix op6program dumper
 * 
 * 2021-2022 (c) Oleg Burdaev
 * mailto: dukesrg@gmail.com
 *
 */
#include <stdio.h>
#include <string.h>

uint32_t parse(FILE *fp, uint32_t size, uint32_t indent, char *parent) {
	char c[4096];
	uint8_t b;
	uint32_t processed = 0, sz, id, data;
        uint32_t index = 1;

	while (processed < size) {
//		printf("%x: ", ftell(fp));
		processed += fread(&b, sizeof(b), 1, fp);
		if ((b & 0x07) == 0x02 && strcmp(parent, "a:1") != 0 && strcmp(parent, "f2:12-12:2-a:1") != 0) {
//			printf("%x ", b);
			id = b;
			if (b >= 0x80) {
				processed += fread(&b, sizeof(b), 1, fp);
				id = (id & 0x7F) | ((uint32_t)b << 7);
//				printf("%x ", b);
			}
			sprintf(c, "%s%s%x:%x", parent, strlen(parent) == 0 ? "" : "-", id, index);
//			printf(c);
		   	processed += fread(&b, sizeof(b), 1, fp);
//			printf("%x ", b);
			sz = b;
			if (b >= 0x80) {
				processed += fread(&b, sizeof(b), 1, fp);
				sz = (sz & 0x7F) | ((uint32_t)b << 7);
//				printf("%x ", b);
			}
			if (sz > 0) {
//				printf("\n");
				processed += parse(fp, sz, indent + 1, c);
			} else {
				printf("%s", parent);
//				for(uint32_t i = 0; i < indent + 1; i++)
//					printf("-");
				printf("-%x -\n", id);
			}
		} else {
			printf("%s", parent);
			if (b == 0x08 || b == 0x10 || b == 0x18 || b == 0x20 || b == 0x28 || b == 0x38) {
				printf("-%x", b);
				processed += fread(&b, sizeof(b), 1, fp);
				data = b;
//				printf(" %x", b);
				for (int i = 1; b >= 0x80; i++) {
					processed += fread(&b, sizeof(b), 1, fp);
//					printf(" %x", b);
					data = (data & ((1 << (7 * i)) - 1)) | ((uint32_t)b << (7 * i));
				}
				printf(" =%x", data);
//				index--;
			} else if (b == 0x0D || b == 0x1D || b == 0x25) {
				printf("-%x", b);
				data = b;
				for(uint32_t i = 0; i < 4; i++) {
					processed += fread(&b, sizeof(b), 1, fp);
//					printf(" %x", b);
					data = (data & ((1 << (8 * i)) - 1)) | ((uint32_t)b << (8 * i));
				}
				printf(" =%f", *(float*)&data);
//				index--;
//			} else if (b == 0x28) {
//				for(uint32_t i = 0; i < indent + 1; i++)
//					printf("-");
//				printf(" %x", b);
//				processed += fread(&b, sizeof(b), 1, fp);
//				printf(" %x", b);
			} else {
//				for(uint32_t i = 0; i < indent; i++)
//					printf("-");
				if (b > 0x20) {
					c[0] = b;
					c[size - processed + 1] = 0;
					processed += fread(&c[1], sizeof(b), size - processed, fp);
					printf(" '%s'", c);
				} else {
					printf(" %x", b);
					while (processed < size) {
						processed += fread(&b, sizeof(b), 1, fp);
						printf(" %x", b);
					}
				}
			}
			printf("\n");
		}
//		printf("%d\n ", processed);
//		printf("\n");
		index++;
	}
	return processed;
}

int main( int argc, char *argv[] ) {
	char c[256] = "";
	uint8_t b;
	uint32_t a, sz;
	FILE *fp = fopen(argv[1], "rb");

	fseek(fp, 4, SEEK_SET);
	fread(&a, sizeof(a), 1, fp);
	fseek(fp, a, SEEK_CUR);
	fread(&a, sizeof(a), 1, fp);
        parse(fp, a, 0, c);

	fclose(fp);
}