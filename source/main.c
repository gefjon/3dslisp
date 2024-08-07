#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <3ds.h>
#include "gc.h"
#include "lisp_object.h"
#include "print.h"

int main(int argc, char* argv[])
{
	gfxInitDefault();
	consoleInit(GFX_TOP, NULL);

	printf("Before gc init...\n");

        gc_init(0x100000);

        printf("After gc init\n");

        lisp_object objs[2] = {0};
        array *arr = allocate_array(NULL, 0, 2, (lisp_object) 0);
        printf("Allocaton 0: raw array is %p\n", arr);
        objs[0] = tag_array(arr);

        printf("After allocation 0\n");
        printf("objs[0] raw = %p\n", objs[0]);
        fputs("objs[0] = ", stdout);
        print_object(stdout, objs[0]);
        putchar('\n');

        objs[1] = tag_string(allocate_string(objs, 1, 16, 'a'));

        printf("After allocation 1\n");
        printf("objs[1] raw = %p\n", objs[1]);
        fputs("objs[1] = ", stdout);
        print_object(stdout, objs[1]);
        putchar('\n');

        array_set_unchecked(lisp_object_as_array(objs[0]), 0, objs[1]);

        printf("After storing 1 into 0[0]\n");
        fputs("objs[0] = ", stdout);
        print_object(stdout, objs[0]);
        putchar('\n');

        lisp_object obj_2 = tag_string(allocate_string(objs, 2, 16, 0));

        printf("After allocation 2\n");

        array_set_unchecked(lisp_object_as_array(objs[0]), 1, obj_2);

        printf("After storing 2 into 0[1]\n");
        fputs("objs[0] = ", stdout);
        print_object(stdout, objs[0]);
        putchar('\n');

        garbage_collect(objs, 2);

        printf("After garbage collection\n");

        printf("objs[0]: %p\n", objs[0]);
        printf("objs[1]: %p\n", objs[1]);
        printf("objs[0][0]: %p\n", array_get_unchecked(lisp_object_as_array(objs[0]), 0));
        printf("objs[0][1]: %p\n", array_get_unchecked(lisp_object_as_array(objs[0]), 1));

        fputs("objs[0] = ", stdout);
        print_object(stdout, objs[0]);
        putchar('\n');

        fputs("objs[1] = ", stdout);
        print_object(stdout, objs[1]);
        putchar('\n');

	// Main loop
	while (aptMainLoop())
	{
		gspWaitForVBlank();
		gfxSwapBuffers();
		hidScanInput();

		// Your code goes here
		u32 kDown = hidKeysDown();
		if (kDown & KEY_START)
			break; // break in order to return to hbmenu
	}

	gfxExit();
	return 0;
}
