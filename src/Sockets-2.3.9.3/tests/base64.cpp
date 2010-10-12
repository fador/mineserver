#include <stdio.h>
#include <cstring>

#include "Base64.h"


int main(int argc,char *argv[])
{
	if (argc < 2)
		return -1;
	std::string str;
	std::string str2;
	Base64 b;
	if (!strcmp(argv[1],"-file"))
	{
		if (argc < 3)
			return -2;
		FILE *fil;
		if ((fil = fopen(argv[2],"rt")) != NULL)
		{
			b.encode(fil, str);
			fclose(fil);
		}
		printf("File:\n%s\n--End of File\n",str.c_str());
		b.decode(str, str2);
		printf("Content:\n%s\n--End of Content\n",str2.c_str());
	}
	else
	{
		b.encode(argv[1], strlen(argv[1]), str);
		printf("'%s' ==> '%s'",argv[1], str.c_str());
		b.decode(str, str2);
		printf(" ==> '%s'\n",str2.c_str());
	}
}
