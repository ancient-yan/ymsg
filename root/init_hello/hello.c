#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

int main(int argc, char** argv)
{

	printf("HHH YYY HHH\n");

//	system("mkdir /data/data/com.browser.txtw/files/yanqiang");

//	mkdir("/data/data/com.browser.txtw/files/yanqiang");

	execl("/system/bin/mount", "mount", "-o", "remount", "/system", (char *) 0);

	return 0;
}