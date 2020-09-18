#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "py/compile.h"
#include "py/runtime.h"
#include "py/repl.h"
#include "py/gc.h"
#include "py/mperrno.h"
#include "py/stackctrl.h"
#include "lib/mp-readline/readline.h"
#include "lib/utils/pyexec.h"
#include "py/mphal.h"
#ifdef OS_USING_VFS
#include "libc_stat.h"
#endif

#include "usr_misc.h"

#ifdef OS_USING_VFS
#include <vfs_posix.h>
#endif


#define MPY_DEBUG		0

void *stack_top = NULL;
static char *heap = NULL;


mp_import_stat_t mp_import_stat(const char *path) {
#ifdef OS_USING_VFS
	struct stat stat;
	if (vfs_file_stat(path, &stat) == 0) {
		if (S_ISDIR(stat.st_mode)) {
			return MP_IMPORT_STAT_DIR;
		} else {
			return MP_IMPORT_STAT_FILE;
		}
	} else {
		return MP_IMPORT_STAT_NO_EXIST;
	}
#endif
}


void nlr_jump_fail(void *val) {
    while (1);
}

void NORETURN __fatal_error(const char *msg) {
    while (1);
}


void Mpy_Task(void* argument)
{
	int stack_dummy;
	os_uint16_t old_flag;
	stack_top = (void *) &stack_dummy;
	
	if (argument){
		if (strlen((char *)argument) < 3){
			argument = NULL;
		}
	}
	
	usr_getchar_init();
	
#if MICROPY_PY_THREAD
	void *stack_addr = usr_GetStack_addr();
	mp_thread_init(stack_addr, ((os_uint32_t)stack_top - (os_uint32_t)stack_addr) / 4);
	
#endif

	
  mp_stack_set_top(stack_top);
  mp_stack_set_limit(usr_GetStack_size() - 1024);


  heap = mp_heap_malloc((size_t)MICROPY_HEAP_ADDR, MP_HEAP_RAM_ADDR);
  if(heap == NULL)
  {
		 printf("heap malloc failed\n");
		 return;
  }
  gc_init(heap, heap + MICROPY_HEAP_SIZE);

  mp_init();
  mp_obj_list_init(MP_OBJ_TO_PTR(mp_sys_path), 0);
  mp_obj_list_append(mp_sys_path, MP_OBJ_NEW_QSTR(MP_QSTR_));
  mp_obj_list_init(MP_OBJ_TO_PTR(mp_sys_argv), 0);

  //printf("mp_init ok\n");
  readline_init0();
  //printf("readline_init0 ok\n");

   /* Save the open flag */
   old_flag = os_console_get_device()->open_flag;
   /* clean the stream flag. stream flag will automatically append '\r' */
   os_console_get_device()->open_flag &= ~OS_DEVICE_FLAG_STREAM;
  
   if (argument) {
	    //os_kprintf(argument);
#ifndef MICROPYTHON_USING_UOS
        os_kprintf("Please enable uos module in sys module option first.\n");
#else
        pyexec_file(argument);
#endif
    } else {
		#if MPY_DEBUG
			if (!access("main.mpy", 0)) {
				if (pyexec_mode_kind == PYEXEC_MODE_FRIENDLY_REPL) {
					pyexec_frozen_module("main.mpy");
			}
		}
			else if (!access("main.py", 0)) {
				if (pyexec_mode_kind == PYEXEC_MODE_FRIENDLY_REPL) {
					pyexec_file("main.py");
			}
		}
		#endif
			
		for(;;)
		{
			if (pyexec_mode_kind == PYEXEC_MODE_RAW_REPL) {
				if (pyexec_raw_repl() != 0) {
					break;
				}
			} else {
				if (pyexec_friendly_repl() != 0) {
					break;
				}
			}
		}
    }

    /* restore the open flag */
    os_console_get_device()->open_flag = old_flag;
  


  gc_sweep_all();
  mp_deinit();
#if MICROPY_PY_THREAD
  mp_thread_deinit();
#endif

  mp_heap_free(heap, MP_HEAP_RAM_ADDR);

  usr_getchar_deinit();
}




