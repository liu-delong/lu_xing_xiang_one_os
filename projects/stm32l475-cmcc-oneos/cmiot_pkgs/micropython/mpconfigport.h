#include <stdint.h>
#include <stdio.h>
#include <board.h>

// options to control how MicroPython is built
//add by zxc
#define MICROPY_KBD_EXCEPTION       (1)
#define MICROPY_HELPER_REPL         (1)
#define MICROPY_PY_DEVICE           (1)
#define MICROPY_PY_THREAD 			(1)
#define MICROPY_PY_THREAD_GIL		(0)

#define MP_ENDIANNESS_LITTLE        (1)
#define MICROPY_PY_NETWORK			(0)
#ifdef MICROPY_USING_AMS
#define MICROPY_PY_USOCKET          (1)
#else 
#define MICROPY_PY_USOCKET          (0)
#endif
#define MICROPY_PY_MACHINE_WDT      (1)
#define MICROPY_QSTR_BYTES_IN_HASH  (1)
#define MICROPY_ALLOC_PATH_MAX      (512)
#define MICROPY_EMIT_X64            (0)
#define MICROPY_EMIT_THUMB          (0)
#define MICROPY_EMIT_INLINE_THUMB   (0)
#define MICROPY_COMP_MODULE_CONST   (0)
#define MICROPY_COMP_CONST          (0)
#define MICROPY_COMP_DOUBLE_TUPLE_ASSIGN (0)
#define MICROPY_COMP_TRIPLE_TUPLE_ASSIGN (0)
#define MICROPY_MEM_STATS           (0)
#define MICROPY_DEBUG_PRINTERS      (1)
#define MICROPY_ENABLE_GC           (1)
#define MICROPY_HELPER_REPL         (1)
#define MICROPY_HELPER_LEXER_UNIX   (0)
#define MICROPY_ENABLE_SOURCE_LINE  (1)
#define MICROPY_ENABLE_DOC_STRING   (0)
#define MICROPY_ERROR_REPORTING     (MICROPY_ERROR_REPORTING_TERSE)
#define MICROPY_BUILTIN_METHOD_CHECK_SELF_ARG (0)
#define MICROPY_PY_ASYNC_AWAIT (0)
#define MICROPY_PY_BUILTINS_BYTEARRAY (1)
#define MICROPY_PY_BUILTINS_DICT_FROMKEYS (0)
#define MICROPY_PY_BUILTINS_STR_COUNT (0)
#define MICROPY_PY_BUILTINS_STR_OP_MODULO (1)
#define MICROPY_PY_GC               (1)
#define MICROPY_PY_MATH             (1)
#define MICROPY_PY_CMATH            (0)
#define MICROPY_PY_IO               (1)
#define MICROPY_PY_SYS              (1)

#define MICROPY_PY_UJSON			 (1)
#define MICROPY_PY_UERRNO 			 (1)
#define MICROPY_PY_UHASHLIB			 (1)

//#define MICROPY_LONGINT_IMPL        (MICROPY_LONGINT_IMPL_NONE)
#define MICROPY_FLOAT_IMPL          (MICROPY_FLOAT_IMPL_DOUBLE)
#define MICROPY_USE_INTERNAL_PRINTF (0)

// control over Python builtins
#define MICROPY_PY_FUNCTION_ATTRS   (1)
#define MICROPY_PY_BUILTINS_STR_UNICODE (1)
#define MICROPY_PY_BUILTINS_STR_CENTER (1)
#define MICROPY_PY_BUILTINS_STR_PARTITION (1)
#define MICROPY_PY_BUILTINS_STR_SPLITLINES (1)
#define MICROPY_PY_BUILTINS_BYTEARRAY (1)
#define MICROPY_PY_BUILTINS_MEMORYVIEW (1)
#define MICROPY_PY_BUILTINS_SLICE_ATTRS (1)
#define MICROPY_PY_ALL_SPECIAL_METHODS (1)
#define MICROPY_PY_BUILTINS_INPUT (1)
#define MICROPY_PY_BUILTINS_POW3 (1)
#define MICROPY_PY_BUILTINS_ENUMERATE (1)
#define MICROPY_PY_BUILTINS_FILTER  (1)
#define MICROPY_PY_BUILTINS_FROZENSET (1)
#define MICROPY_PY_BUILTINS_REVERSED (1)
#define MICROPY_PY_BUILTINS_SET     (1)
#define MICROPY_PY_BUILTINS_HELP    (1)

#define MICROPY_PY_BUILTINS_HELP_MODULES (1)
#define MICROPY_PY_BUILTINS_SLICE   (1)
#define MICROPY_PY_BUILTINS_PROPERTY (1)
#define MICROPY_PY_BUILTINS_MIN_MAX (1)
#define MICROPY_PY___FILE__         (1)
#define MICROPY_PY_GC               (1)
#define MICROPY_PY_ARRAY            (1)
#define MICROPY_PY_ARRAY_SLICE_ASSIGN (1)
#define MICROPY_PY_ATTRTUPLE        (1)
#define MICROPY_PY_COLLECTIONS      (1)
#define MICROPY_PY_COLLECTIONS_ORDEREDDICT (1)
#define MICROPY_PY_MATH             (1)
#define MICROPY_PY_MATH_SPECIAL_FUNCTIONS (1)
#define MICROPY_PY_MICROPYTHON_MEM_INFO (1)
#define MICROPY_STREAMS_NON_BLOCK   (1)
#define MICROPY_MODULE_WEAK_LINKS   (1)
#define MICROPY_CAN_OVERRIDE_BUILTINS (1)
#define MICROPY_USE_INTERNAL_ERRNO  (1)
#define MICROPY_USE_INTERNAL_PRINTF (0)
#define MICROPY_PY_STRUCT           (1)
#define MICROPY_PY_SYS              (1)
#define MICROPY_MODULE_FROZEN_MPY   (1)
#define MICROPY_CPYTHON_COMPAT      (1)
#define MICROPY_LONGINT_IMPL        (MICROPY_LONGINT_IMPL_MPZ)
#define MICROPY_FLOAT_IMPL          (MICROPY_FLOAT_IMPL_DOUBLE)
#define MICROPY_READER_VFS          (0)

#define MICROPY_PY_ONENET			(0)
#define MICROPY_DUMP_ADDR			(1)

#define MICROPY_PY_PIN              (1)
#define MICROPY_PY_UART             (1)
#define MICROPY_PY_I2C              (1)
#define MICROPY_PY_SPI				(1)
#define MICROPY_PY_PWM				(1)
#define MICROPY_PY_CAN				(1)
#define MICROPY_PY_DAC				(1)

#define MICROPY_PY_OS_DUPTERM       (0)
#define MICROPY_VFS                 (0)
#define MICROPY_VFS_FAT             (0)
#define MICROPY_PY_UTIME            (1)
#define MICROPY_PY_MACHINE          (1)
#define MICROPY_PY_MACHINE_PIN_MAKE_NEW mp_pin_make_new
#define MICROPY_PY_UTIME_MP_HAL     (1)
#define MICROPY_PY_UTIMEQ           (0)

/*****************************************************************************/

// type definitions for the specific machine
#define MICROPYTHON_USING_UOS

#ifdef MICROPYTHON_USING_UOS
#define MICROPY_PY_IO_FILEIO         (1)
#define MICROPY_PY_MODUOS            (1)
#define MICROPY_PY_MODUOS_FILE       (1)
#define MICROPY_PY_SYS_STDFILES      (1)
#define MICROPY_READER_POSIX         (1)
#define MICROPY_PY_BUILTINS_COMPILE  (1)
#define MICROPY_PY_BUILTINS_EXECFILE (1)
#endif


#define MICROPY_MAKE_POINTER_CALLABLE(p) ((void*)((mp_uint_t)(p) | 1))

#define MICROPY_HEAP_SIZE	(MICROPYTHON_RAM_SIZE * 1024)
#define MICROPY_HEAP_ADDR	MICROPYTHON_RAM_START //(MICROPYTHON_RAM_START + MICROPYTHON_RAM_SIZE * 1024)


#define MICROPY_STACK_SIZE	 1024

#define UINT_FMT "%lu"
#define INT_FMT "%ld"

#ifndef errno
#define errno
#endif

typedef int32_t mp_int_t; // must be pointer size
typedef uint32_t mp_uint_t; // must be pointer size
typedef long mp_off_t;


// dummy print
#define MP_PLAT_PRINT_STRN(str, len) mp_hal_stdout_tx_strn_stream(str, len)

// extra built in names to add to the global namespace
#define MICROPY_PORT_BUILTINS \
    { MP_ROM_QSTR(MP_QSTR_open), MP_ROM_PTR(&mp_builtin_open_obj) },

// We need to provide a declaration/definition of alloca()

//by zxc
#define MICROPY_HW_BOARD_NAME          "MicroPython board"
#define MICROPY_HW_MCU_NAME            "stm32l475"

//use pin module
#define MICROPY_PY_MACHINE_PIN 		(1)

//use low power
#define MICROPY_PY_LPOWER      		(1)

#define MICROPY_PY_MACHINE_PM		(1)

//use rtc module
#define MICROPY_PY_MACHINE_RTC		(1)

//use i2c
#ifndef MICROPY_PY_MACHINE_I2C
#define MICROPY_PY_MACHINE_I2C      (1)
#define MICROPY_PY_MACHINE_I2C_MAKE_NEW machine_hard_i2c_make_new
#endif

#ifndef MICROPY_PY_MACHINE_SPI
#define MICROPY_PY_MACHINE_SPI (1)
#endif

#ifndef MICROPY_PY_MACHINE_CAN
#define MICROPY_PY_MACHINE_CAN (1)
#endif

#ifndef MICROPY_PY_MACHINE_UART
#define MICROPY_PY_MACHINE_UART (1)
#endif

//use adc module
#ifndef MICROPY_PY_MACHINE_ADC
#define MICROPY_PY_MACHINE_ADC (1)
#endif

#ifndef MICROPY_PY_MACHINE_DAC
#define MICROPY_PY_MACHINE_DAC (1)
#endif

#ifndef MICROPY_PY_MACHINE_PWM
#define MICROPY_PY_MACHINE_PWM (1)
#endif

extern const struct _mp_obj_module_t mp_module_os;
#define MODUOS_PORT_BUILTIN_MODULES         { MP_ROM_QSTR(MP_QSTR_uos), MP_ROM_PTR(&mp_module_os ) },


#if MICROPY_PY_UERRNO
extern const struct _mp_obj_module_t mp_module_uerrno;
//#define MODUERRNO_PORT_BUILTIN_MODULE_WEAK_LINKS           { MP_ROM_QSTR(MP_QSTR_errno), MP_ROM_PTR(&mp_module_uerrno ) },
#endif

#if MICROPY_PY_DEVICE
extern const struct _mp_obj_module_t mp_module_device;
#define MODUDEVICE_PORT_BUILTIN_MODULES            { MP_ROM_QSTR(MP_QSTR_device), MP_ROM_PTR(&mp_module_device ) },
#endif


#if MICROPY_PY_UJSON
extern const struct _mp_obj_module_t mp_module_ujson;
//#define MODUJSON_PORT_BUILTIN_MODULE_WEAK_LINKS            { MP_ROM_QSTR(MP_QSTR_json), MP_ROM_PTR(&mp_module_ujson ) },
#endif

#if MICROPY_PY_UHASHLIB
extern const struct _mp_obj_module_t mp_module_uhashlib;
//#define MODUHASHLIB_PORT_BUILTIN_MODULE_WEAK_LINKS         { MP_ROM_QSTR(MP_QSTR_hashlib), MP_ROM_PTR(&mp_module_uhashlib ) },
#endif

#if MICROPY_DUMP_ADDR
extern const struct _mp_obj_module_t mp_module_dumpaddr;
#define MODDUMPADDR_PORT_BUILTIN_MODULES { MP_ROM_QSTR(MP_QSTR_dumpaddr), MP_ROM_PTR(&mp_module_dumpaddr) },
#endif

#if MICROPY_PY_LPOWER
extern const struct _mp_obj_module_t mp_module_lpower;
#define MODULPOWER_PORT_BUILTIN_MODULES { MP_ROM_QSTR(MP_QSTR_Lpower), MP_ROM_PTR(&mp_module_lpower) },

#endif

#if MICROPY_PY_USOCKET
extern const struct _mp_obj_module_t mp_module_usocket;
#define MODUSOCKET_PORT_BUILTIN_MODULES { MP_ROM_QSTR(MP_QSTR_usocket), MP_ROM_PTR(&mp_module_usocket)},
#else 
#define MODUSOCKET_PORT_BUILTIN_MODULES 
#endif


#if MICROPY_PY_UTIME
extern const struct _mp_obj_module_t mp_module_time;
#define MODUTIME_PORT_BUILTIN_MODULES		{ MP_ROM_QSTR(MP_QSTR_utime), MP_ROM_PTR(&mp_module_time)},
#endif

#if MICROPY_PY_ONENET
extern const struct _mp_obj_module_t mp_module_onenet;
#define MODUTIME_ONENET_BUILTIN_MODULES		{ MP_ROM_QSTR(MP_QSTR_OneNET), MP_ROM_PTR(&mp_module_onenet)},
#else
#define MODUTIME_ONENET_BUILTIN_MODULES
#endif

extern const struct _mp_obj_module_t pyb_module;


#define MICROPY_PORT_BUILTIN_MODULES \
		{ MP_ROM_QSTR(MP_QSTR_machine), MP_ROM_PTR(&pyb_module) }, \
		MODULPOWER_PORT_BUILTIN_MODULES \
		MODUTIME_PORT_BUILTIN_MODULES	\
		MODUOS_PORT_BUILTIN_MODULES \
		MODDUMPADDR_PORT_BUILTIN_MODULES \
		MODUDEVICE_PORT_BUILTIN_MODULES \
		MODUSOCKET_PORT_BUILTIN_MODULES	\
		MODUTIME_ONENET_BUILTIN_MODULES \
		
		
		
#define MP_STATE_PORT                  MP_STATE_VM

#define MICROPY_PORT_ROOT_POINTERS     const char *readline_hist[8];

#if MICROPY_PY_THREAD
#define MICROPY_EVENT_POLL_HOOK \
    do { \
        extern void mp_handle_pending(void); \
        mp_handle_pending(); \
        MP_THREAD_GIL_EXIT(); \
        MP_THREAD_GIL_ENTER(); \
    } while (0);
#else
#define MICROPY_EVENT_POLL_HOOK \
    do { \
        extern void mp_handle_pending(void); \
        mp_handle_pending(); \
    } while (0);
#endif


//#define MICROPY_PORT_BUILTIN_MODULE_WEAK_LINKS \
//			
#include <alloca.h>
