#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include "HogeDriver.h"
#include "FAT12.h"

int main(int argc, char *argv[]) {

    DiskDriver* driver = new HogeDriver("./floppy.dat", 50);
    FAT12*      fat    = new FAT12(driver);

    if (!fat->initilize()) {

        int errorNo = fat->getErrorNo();

        if (errorNo == FAT12::BPB_ERROR) printf("BPB read  error \n");
        else if (errorNo == FAT12::NOT_FAT12_ERROR) printf("NOT FAT12 error \n");
        else if (errorNo == FAT12::FAT_READ_ERROR) printf("NOT FAT12 error \n");
        else printf("unknown error \n");

        /* error */
        return -1;
    }

    printf("fat initilize\n");

    delete fat;
    delete driver;

    printf("size of FTIME = %d FDATE = %d", sizeof(FTIME), sizeof(FDATE));

    return 0;
}
