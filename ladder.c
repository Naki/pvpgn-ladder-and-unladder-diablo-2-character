// Copyright faster. All rights reserved.

#include <stdio.h>
#include <stdlib.h>

#define D2CHARSAVE_VERSION_OFFSET 0x04
#define D2CHARSAVE_CHECKSUM_OFFSET 0x0C
#define D2CHARSAVE_STATUS_OFFSET_109 0x24
#define D2CHARINFO_STATUS_FLAG_LADDER 0x40

#define tf(a) ((a)?1:0)

#define charstatus_set_ladder(status) ( status |= (D2CHARINFO_STATUS_FLAG_LADDER))
#define charstatus_switch_ladder(status) ( status ^= (D2CHARINFO_STATUS_FLAG_LADDER))
#define charstatus_get_ladder(status) tf( status & D2CHARINFO_STATUS_FLAG_LADDER)

static int d2charsave_checksum(unsigned char const *data, unsigned int len, unsigned int offset)
{
	int checksum;
	unsigned int i;
	unsigned int ch;

	if (!data) return 0;
	checksum=0;
	for (i=0; i<len; i++) {
		ch=data[i];
		if (i>=offset && i<offset+sizeof(int)) ch=0;
		ch+=(checksum<0);
		checksum=2*checksum+ch;
	}
	return checksum;
}

static unsigned char buf[65536];

int main(int argc, char **argv)
{
	FILE *fp;
	unsigned int checksum;
	unsigned int size;
	unsigned char status;

	if (argc!=2) {
		printf("%s <charsave>\n", argv[0]);
		return 0;
	}
	fp = fopen(argv[1], "rb+");
	if (!fp) {
		printf("failed open file\n");
		return -1;
	}
	size = fread(buf, 1, sizeof(buf), fp);
	if (!feof(fp)) {
		printf("error reading file\n");
		fclose(fp);
		return -2;
	}
	if (buf[D2CHARSAVE_VERSION_OFFSET]<0x5c) {
		printf("wrong version number\n");
		fclose(fp);
		return -3;
	}
	status = buf[D2CHARSAVE_STATUS_OFFSET_109];
	charstatus_switch_ladder(status);
	buf[D2CHARSAVE_STATUS_OFFSET_109] = status;
	checksum = d2charsave_checksum(buf, size, D2CHARSAVE_CHECKSUM_OFFSET);
	*((unsigned int*)(buf+D2CHARSAVE_CHECKSUM_OFFSET)) = checksum;
	fseek(fp, 0, SEEK_SET);
	if (fwrite(buf, 1, size, fp)!=size) {
		printf("error writing save file\n");
		fclose(fp);
		return -4;
	}
	fclose(fp);
	if (charstatus_get_ladder(status))
		printf("set to LADDER char.\n");
	else
		printf("set to none-LADDER char.\n");
	return 0;
}