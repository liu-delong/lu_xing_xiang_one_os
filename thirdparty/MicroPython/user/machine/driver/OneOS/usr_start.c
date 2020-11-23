#include "usr_misc.h"
#include "shell.h"
#include "os_util.h"
#include "string.h"
#ifdef MICROPY_USING_AMS
#include <vfs_fs.h>
#include "ams.h"
#endif

int run_mpy(int argc, char **argv)
{
	char *file;
	int length = 0;
    if (argc != 2)
    {
		os_kprintf("parameter is wrong\n");
        return 0;
    }
	file = argv[1];
	length = strlen(file);
	if (strncmp(file+length-3,".py", 3) !=0){
		os_kprintf("file is wrong\n");
		return -1;
	}
	Mpy_Task(file);
	return 0;
}

SH_CMD_EXPORT(mpy, run_mpy, "run py file");


#ifdef MICROPY_USING_AMS
static int fs_dev_link(void)
{
	
	char *file_sys_device =  MICROPY_FS_DEVICE_NAME; //"W25Q64" or sd0;
    /* Mount the file system from tf card */
    if (vfs_mount(file_sys_device, "/", "fat", 0, 0) == 0)
    {
        os_kprintf("Filesystem initialized!\n");
		os_task_mdelay(500);
    }
    else
    {
        os_kprintf("Failed to initialize filesystem!\n");
		
		os_task_delay(1);
		if(vfs_mkfs("fat" ,file_sys_device) < 0)
		{
			os_kprintf("vfs_mkfs failed");
		}
		vfs_mount(file_sys_device, "/", "fat", 0, 0);
		return 1;
    }

    return 0;
}

static int micropy_start(void)
{
	fs_dev_link();

	setup_ams_thread();

	return 0;
}
OS_APP_INIT(micropy_start);
#endif
OS_BOARD_INIT(Init_listhead);

SH_CMD_EXPORT(Mpy_Task, Mpy_Task, "Run MicroPython");
