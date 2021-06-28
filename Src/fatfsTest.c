#include "misra.h"

MISRAC_DISABLE
#include "ffconf.h"
#include "ospi_nor_mx25l25645.h"
#define FF_MAX_SS _MAX_SS
#define FF_MIN_SS _MIN_SS

#if 0
// From http://elm-chan.org/fsw/ff/res/app4.c

/*----------------------------------------------------------------------/
/ Low level disk I/O module function checker                            /
/-----------------------------------------------------------------------/
/ WARNING: The data on the target drive will be lost!
*/

#include <stdio.h>
#include <string.h>
#include "ff.h"         /* Declarations of sector size */
#include "diskio.h"     /* Declarations of disk functions */



static DWORD pn (       /* Pseudo random number generator */
    DWORD pns   /* 0:Initialize, !0:Read */
)
{
    static DWORD lfsr;
    UINT n;


    if (pns) {
        lfsr = pns;
        for (n = 0; n < 32; n++) pn(0);
    }
    if (lfsr & 1) {
        lfsr >>= 1;
        lfsr ^= 0x80200003;
    } else {
        lfsr >>= 1;
    }
    return lfsr;
}

int test_diskio (
    BYTE pdrv,      /* Physical drive number to be checked (all data on the drive will be lost) */
    UINT ncyc,      /* Number of test cycles */
    DWORD* buff,    /* Pointer to the working buffer */
    UINT sz_buff    /* Size of the working buffer in unit of byte */
)
{
    UINT n, cc, ns;
    DWORD sz_drv, lba, lba2, sz_eblk, pns = 1;
    WORD sz_sect;
    BYTE *pbuff = (BYTE*)buff;
    DSTATUS ds;
    DRESULT dr;


    printf("test_diskio(%u, %u, 0x%08X, 0x%08X)\n", pdrv, ncyc, (UINT)buff, sz_buff);

    if (sz_buff < FF_MAX_SS + 8) {
        printf("Insufficient work area to run the program.\n");
        return 1;
    }

    for (cc = 1; cc <= ncyc; cc++) {
        printf("**** Test cycle %u of %u start ****\n", cc, ncyc);

        printf(" disk_initalize(%u)", pdrv);
        ds = disk_initialize(pdrv);
        if (ds & STA_NOINIT) {
            printf(" - failed.\n");
            return 2;
        } else {
            printf(" - ok.\n");
        }

        printf("**** Get drive size ****\n");
        printf(" disk_ioctl(%u, GET_SECTOR_COUNT, 0x%08X)", pdrv, (UINT)&sz_drv);
        sz_drv = 0;
        dr = disk_ioctl(pdrv, GET_SECTOR_COUNT, &sz_drv);
        if (dr == RES_OK) {
            printf(" - ok.\n");
        } else {
            printf(" - failed.\n");
            return 3;
        }
        if (sz_drv < 128) {
            printf("Failed: Insufficient drive size to test.\n");
            return 4;
        }
        printf(" Number of sectors on the drive %u is %lu.\n", pdrv, sz_drv);

#if FF_MAX_SS != FF_MIN_SS
        printf("**** Get sector size ****\n");
        printf(" disk_ioctl(%u, GET_SECTOR_SIZE, 0x%X)", pdrv, (UINT)&sz_sect);
        sz_sect = 0;
        dr = disk_ioctl(pdrv, GET_SECTOR_SIZE, &sz_sect);
        if (dr == RES_OK) {
            printf(" - ok.\n");
        } else {
            printf(" - failed.\n");
            return 5;
        }
        printf(" Size of sector is %u bytes.\n", sz_sect);
#else
        sz_sect = FF_MAX_SS;
#endif

        printf("**** Get block size ****\n");
        printf(" disk_ioctl(%u, GET_BLOCK_SIZE, 0x%X)", pdrv, (UINT)&sz_eblk);
        sz_eblk = 0;
        dr = disk_ioctl(pdrv, GET_BLOCK_SIZE, &sz_eblk);
        if (dr == RES_OK) {
            printf(" - ok.\n");
        } else {
            printf(" - failed.\n");
        }
        if (dr == RES_OK || sz_eblk >= 2) {
            printf(" Size of the erase block is %lu sectors.\n", sz_eblk);
        } else {
            printf(" Size of the erase block is unknown.\n");
        }

        /* Single sector write test */
        printf("**** Single sector write test ****\n");
        lba = 0;
        for (n = 0, pn(pns); n < sz_sect; n++) pbuff[n] = (BYTE)pn(0);
        printf(" disk_write(%u, 0x%X, %lu, 1)", pdrv, (UINT)pbuff, lba);
        dr = disk_write(pdrv, pbuff, lba, 1);
        if (dr == RES_OK) {
            printf(" - ok.\n");
        } else {
            printf(" - failed.\n");
            return 6;
        }
        printf(" disk_ioctl(%u, CTRL_SYNC, NULL)", pdrv);
        dr = disk_ioctl(pdrv, CTRL_SYNC, 0);
        if (dr == RES_OK) {
            printf(" - ok.\n");
        } else {
            printf(" - failed.\n");
            return 7;
        }
        memset(pbuff, 0, sz_sect);
        printf(" disk_read(%u, 0x%X, %lu, 1)", pdrv, (UINT)pbuff, lba);
        dr = disk_read(pdrv, pbuff, lba, 1);
        if (dr == RES_OK) {
            printf(" - ok.\n");
        } else {
            printf(" - failed.\n");
            return 8;
        }
        for (n = 0, pn(pns); n < sz_sect && pbuff[n] == (BYTE)pn(0); n++) ;
        if (n == sz_sect) {
            printf(" Read data matched.\n");
        } else {
            printf(" Read data differs from the data written.\n");
            return 10;
        }
        pns++;

        printf("**** Multiple sector write test ****\n");
        lba = 5; ns = sz_buff / sz_sect;
        if (ns > 4) ns = 4;
        if (ns > 1) {
            for (n = 0, pn(pns); n < (UINT)(sz_sect * ns); n++) pbuff[n] = (BYTE)pn(0);
            printf(" disk_write(%u, 0x%X, %lu, %u)", pdrv, (UINT)pbuff, lba, ns);
            dr = disk_write(pdrv, pbuff, lba, ns);
            if (dr == RES_OK) {
                printf(" - ok.\n");
            } else {
                printf(" - failed.\n");
                return 11;
            }
            printf(" disk_ioctl(%u, CTRL_SYNC, NULL)", pdrv);
            dr = disk_ioctl(pdrv, CTRL_SYNC, 0);
            if (dr == RES_OK) {
                printf(" - ok.\n");
            } else {
                printf(" - failed.\n");
                return 12;
            }
            memset(pbuff, 0, sz_sect * ns);
            printf(" disk_read(%u, 0x%X, %lu, %u)", pdrv, (UINT)pbuff, lba, ns);
            dr = disk_read(pdrv, pbuff, lba, ns);
            if (dr == RES_OK) {
                printf(" - ok.\n");
            } else {
                printf(" - failed.\n");
                return 13;
            }
            for (n = 0, pn(pns); n < (UINT)(sz_sect * ns) && pbuff[n] == (BYTE)pn(0); n++) ;
            if (n == (UINT)(sz_sect * ns)) {
                printf(" Read data matched.\n");
            } else {
                printf(" Read data differs from the data written.\n");
                return 14;
            }
        } else {
            printf(" Test skipped.\n");
        }
        pns++;

        printf("**** Single sector write test (unaligned buffer address) ****\n");
        lba = 5;
        for (n = 0, pn(pns); n < sz_sect; n++) pbuff[n+3] = (BYTE)pn(0);
        printf(" disk_write(%u, 0x%X, %lu, 1)", pdrv, (UINT)(pbuff+3), lba);
        dr = disk_write(pdrv, pbuff+3, lba, 1);
        if (dr == RES_OK) {
            printf(" - ok.\n");
        } else {
            printf(" - failed.\n");
            return 15;
        }
        printf(" disk_ioctl(%u, CTRL_SYNC, NULL)", pdrv);
        dr = disk_ioctl(pdrv, CTRL_SYNC, 0);
        if (dr == RES_OK) {
            printf(" - ok.\n");
        } else {
            printf(" - failed.\n");
            return 16;
        }
        memset(pbuff+5, 0, sz_sect);
        printf(" disk_read(%u, 0x%X, %lu, 1)", pdrv, (UINT)(pbuff+5), lba);
        dr = disk_read(pdrv, pbuff+5, lba, 1);
        if (dr == RES_OK) {
            printf(" - ok.\n");
        } else {
            printf(" - failed.\n");
            return 17;
        }
        for (n = 0, pn(pns); n < sz_sect && pbuff[n+5] == (BYTE)pn(0); n++) ;
        if (n == sz_sect) {
            printf(" Read data matched.\n");
        } else {
            printf(" Read data differs from the data written.\n");
            return 18;
        }
        pns++;

        printf("**** 4GB barrier test ****\n");
        if (sz_drv >= 128 + 0x80000000 / (sz_sect / 2)) {
            lba = 6; lba2 = lba + 0x80000000 / (sz_sect / 2);
            for (n = 0, pn(pns); n < (UINT)(sz_sect * 2); n++) pbuff[n] = (BYTE)pn(0);
            printf(" disk_write(%u, 0x%X, %lu, 1)", pdrv, (UINT)pbuff, lba);
            dr = disk_write(pdrv, pbuff, lba, 1);
            if (dr == RES_OK) {
                printf(" - ok.\n");
            } else {
                printf(" - failed.\n");
                return 19;
            }
            printf(" disk_write(%u, 0x%X, %lu, 1)", pdrv, (UINT)(pbuff+sz_sect), lba2);
            dr = disk_write(pdrv, pbuff+sz_sect, lba2, 1);
            if (dr == RES_OK) {
                printf(" - ok.\n");
            } else {
                printf(" - failed.\n");
                return 20;
            }
            printf(" disk_ioctl(%u, CTRL_SYNC, NULL)", pdrv);
            dr = disk_ioctl(pdrv, CTRL_SYNC, 0);
            if (dr == RES_OK) {
            printf(" - ok.\n");
            } else {
                printf(" - failed.\n");
                return 21;
            }
            memset(pbuff, 0, sz_sect * 2);
            printf(" disk_read(%u, 0x%X, %lu, 1)", pdrv, (UINT)pbuff, lba);
            dr = disk_read(pdrv, pbuff, lba, 1);
            if (dr == RES_OK) {
                printf(" - ok.\n");
            } else {
                printf(" - failed.\n");
                return 22;
            }
            printf(" disk_read(%u, 0x%X, %lu, 1)", pdrv, (UINT)(pbuff+sz_sect), lba2);
            dr = disk_read(pdrv, pbuff+sz_sect, lba2, 1);
            if (dr == RES_OK) {
                printf(" - ok.\n");
            } else {
                printf(" - failed.\n");
                return 23;
            }
            for (n = 0, pn(pns); pbuff[n] == (BYTE)pn(0) && n < (UINT)(sz_sect * 2); n++) ;
            if (n == (UINT)(sz_sect * 2)) {
                printf(" Read data matched.\n");
            } else {
                printf(" Read data differs from the data written.\n");
                return 24;
            }
        } else {
            printf(" Test skipped.\n");
        }
        pns++;

        printf("**** Test cycle %u of %u completed ****\n\n", cc, ncyc);
    }

    return 0;
}

DWORD buff[FF_MAX_SS];  /* Working buffer (4 sector in size) */
int rc;
FATFS fs;           /* Filesystem object */
FIL fil;            /* File object */
FRESULT fres;        /* API result code */
FILINFO fno;
DWORD fre_clust, fre_sect, tot_sect;
uint8_t SD_Buffer[15];
int operation = 0;
//static uint8_t buffer[_MAX_SS];

int test_fatFS(void)
{
    switch (operation)
    {
    case 1:
        /* Check function/compatibility of the physical drive #0 */
        rc = test_diskio(0, 3, buff, sizeof buff);

        if (rc) {
            printf("Sorry the function/compatibility test failed. (rc=%d)\nFatFs will not work with this disk driver.\n", rc);
        } else {
            printf("Congratulations! The disk driver works well.\n");
        }
        break;

    case 2:
        /* Mount FATFS */
        fres = f_mount(&fs, "", 0);
        
        /* Create FAT volume with default cluster size */
        fres = f_mkfs("", FM_ANY, 0, buff, sizeof(buff));
        
        /* Set drive label */
        fres = f_setlabel("FATFS");
        
        /* Get volume information and free clusters of drive 1 - may need scaling to suit */
        FATFS *pFs;
        pFs = &fs;
        fres = f_getfree("", &fre_clust, &pFs);
        tot_sect = (fs.n_fatent - 2) * fs.csize;
        fre_sect = fre_clust * fs.csize;
    
        /* Create file */
        fres = f_open(&fil, "file.txt", FA_CREATE_ALWAYS | FA_READ | FA_WRITE);
        
        /* Write data */
        fres = f_write(&fil, "Hello world", 11, NULL);
        
        /* Close file */
        fres = f_close(&fil);

        /* Create directory */
        fres = f_mkdir("folder");
        
        /* Unmount */
        f_mount(NULL, "", 1);

        break;
      
    case 3:
        /* Mount FATFS */
        fres = f_mount(&fs, "", 0);
        
        /* No need to call MKFS here again because fat system is already on flash */
        
        /* Get file information - must be before file is opened */
        fres = f_stat("file.txt", &fno);

        /* Open file */
        fres = f_open(&fil, "file.txt", FA_READ | FA_WRITE);
        
        /* Read data */
        fres = f_read(&fil, SD_Buffer, 11, NULL);
        
        /* Close file */
        fres = f_close(&fil);
        
        /* Delete directory */
        if (f_stat("folder", NULL) == FR_OK)
        {
            fres = f_unlink("folder");
        }
        
        /* Unmount */
        f_mount(NULL, "", 1);
      break;       
      
    case 4:
        OSPI_NOR_SelfTest();
        break;
    }
    
    return rc;
}

#endif
MISRAC_ENABLE