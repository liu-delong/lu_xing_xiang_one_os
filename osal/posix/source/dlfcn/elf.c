/**
 ***********************************************************************************************************************
 * Copyright (c) 2020, China Mobile Communications Group Co.,Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with 
 * the License. You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on 
 * an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the 
 * specific language governing permissions and limitations under the License.
 *
 * @file        elf.c
 *
 * @brief      This file provides routines to handle ELF format object files.The object files are loaded into memory;
 *             relocations are applied;Elf files are used by the system as kernel modules.
 *             
 *
 * @revision
 * Date         Author          Notes
 * 2020-06-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */


#include <os_module.h>
#include <os_assert.h>
#include <os_errno.h>
#include <vfs_posix.h>

#include <string.h>
#include <stdlib.h>



#include <dlfcn.h>
#include "elf.h"

#define DBG_EXT_LVL         DBG_EXT_DEBUG
#define DBG_EXT_TAG         "elf"

#include <os_dbg_ext.h>         

/*Perform relocation  for rel or rela segments.Depend on different architectures*/
extern int elf_arch_relocate(struct os_module *module, Elf32_Rel *rel, Elf32_Addr sym_val);

/**
 ***********************************************************************************************************************
 * @brief           This function load ELF shared object
 *
 * @param[out]      module          Pointer to kernel module
 * @param[in]       elf_ptr         Pointer to ELF file.
 *
 * @return          On success, return OS_EOK; on error, return OS_ERROR.
 * @retval          OS_EOK          Load ELF shared object successfully.
 * @retval          OS_ERROR        Load ELF shared object error.
 ***********************************************************************************************************************
 */
os_err_t elf_load_shared_object(struct os_module *module, void *elf_ptr)
{
    os_bool_t linked;
    os_uint32_t index;
    os_uint32_t module_size;    
    Elf32_Addr vstart_addr;
    Elf32_Addr vend_addr;
    os_bool_t has_vstart;

    os_bool_t unsolved;

    OS_ASSERT(elf_ptr != OS_NULL);

    linked = OS_FALSE;
    if (memcmp(ehdr->e_ident, RTMMAG, SELFMAG) == 0)
    {
        /* rtmlinker finished */
        linked = OS_TRUE;
    }

    /* get the ELF image size */
    has_vstart = OS_FALSE;
    vstart_addr = vend_addr = 0;
    for (index = 0; index < ehdr->e_phnum; index++)
    {
        if (phdr[index].p_type != PT_LOAD)
        {
            continue;
        }

        LOG_EXT_D("LOAD segment: %d, 0x%p, 0x%08x", index, phdr[index].p_vaddr, phdr[index].p_memsz);

        if (phdr[index].p_memsz < phdr[index].p_filesz)
        {
            LOG_EXT_E("invalid elf: segment %d: p_memsz: %d, p_filesz: %d\n",
                       index, phdr[index].p_memsz, phdr[index].p_filesz);
            return  OS_ERROR;
        }
     
        if (!has_vstart)
        {
            vstart_addr = phdr[index].p_vaddr;
            vend_addr = phdr[index].p_vaddr + phdr[index].p_memsz;
            has_vstart = OS_TRUE;
            if (vend_addr < vstart_addr)
            {
                LOG_EXT_E("invalid elf: segment %d: p_vaddr: %d, p_memsz: %d\n",
                           index, phdr[index].p_vaddr, phdr[index].p_memsz);
                return OS_ERROR;
            }
        }
        else
        {
            if (phdr[index].p_vaddr < vend_addr)
            {
                LOG_EXT_E("invalid elf: segment should be sorted and not overlapped\n");
                return OS_ERROR;
            }
            if (phdr[index].p_vaddr > vend_addr + 16)
            {
                /* There should not be too much padding in the object files. */
                LOG_EXT_W("warning: too much padding before segment %d", index);
            }

            vend_addr = phdr[index].p_vaddr + phdr[index].p_memsz;
            if (vend_addr < phdr[index].p_vaddr)
            {
                LOG_EXT_E("invalid elf: "
                           "segment %d address overflow\n", index);
                return OS_ERROR;
            }
        }
    }

    module_size = vend_addr - vstart_addr;
    LOG_EXT_D("module size: %d, vstart_addr: 0x%p", module_size, vstart_addr);
    if (module_size == 0)
    {
        LOG_EXT_E("elf_size  error\n");
        return OS_ERROR;
    }

    module->vstart_addr = vstart_addr;
    module->nref = 0;

    /* allocate module space */
    module->mem_space = os_malloc(module_size);
    if (module->mem_space == OS_NULL)
    {
        LOG_EXT_E("allocate space failed.\n");
        return OS_ERROR;
    }
    module->mem_size = module_size;

    /* zero all space */
    memset(module->mem_space, 0, module_size);
    for (index = 0; index < ehdr->e_phnum; index++)
    {
        if (phdr[index].p_type == PT_LOAD)
        {
            memcpy((char *)module->mem_space + phdr[index].p_vaddr - vstart_addr,
                   (os_uint8_t *)ehdr + phdr[index].p_offset,
                   phdr[index].p_filesz);
        }
    }

    /* set module entry */
    module->entry_addr = (os_module_entry_func_t)((char *)module->mem_space + ehdr->e_entry - vstart_addr);
    unsolved = OS_FALSE;
    /* handle relocation section */
    for (index = 0; index < ehdr->e_shnum; index ++)
    {
        os_uint32_t i, nr_reloc;
        Elf32_Sym *symtab;
        Elf32_Rel *rel;
        os_uint8_t *strtab;

        if (!IS_REL(shdr[index]))
        {
            continue;
        }
        
        /* get relocate item */
        rel = (Elf32_Rel *)((os_uint8_t *)elf_ptr + shdr[index].sh_offset);

        /* locate .rel.plt and .rel.dyn section */
        symtab = (Elf32_Sym *)((os_uint8_t *)elf_ptr +
                               shdr[shdr[index].sh_link].sh_offset);
        strtab = (os_uint8_t *)elf_ptr + shdr[shdr[shdr[index].sh_link].sh_link].sh_offset;
        nr_reloc = (os_uint32_t)(shdr[index].sh_size / sizeof(Elf32_Rel));

        /* relocate every items */
        for (i = 0; i < nr_reloc; i ++)
        {
            Elf32_Sym *sym = &symtab[ELF32_R_SYM(rel->r_info)];

            LOG_EXT_D("relocate symbol %s shndx %d", strtab + sym->st_name, sym->st_shndx);

            if ((sym->st_shndx != SHT_NULL) ||(ELF_ST_BIND(sym->st_info) == STB_LOCAL))
            {
                Elf32_Addr addr;

                addr = (Elf32_Addr)((char *)module->mem_space + sym->st_value - vstart_addr);
                elf_arch_relocate(module, rel, addr);
            }
            else if (!linked)
            {
                Elf32_Addr addr;

                LOG_EXT_D("relocate symbol: %s", strtab + sym->st_name);
                /* need to resolve symbol in kernel symbol table */
                addr = os_module_symbol_find((const char *)(strtab + sym->st_name));
                if (addr == 0)
                {
                    LOG_EXT_E("ELF: can't find %s in kernel symbol table", strtab + sym->st_name);
                    unsolved = OS_TRUE;
                }
                else
                {
                    elf_arch_relocate(module, rel, addr);
                }
            }
            rel ++;
        }

        if (unsolved)
        {
            return OS_ERROR;
        }
    }

    /* construct module symbol table */
    for (index = 0; index < ehdr->e_shnum; index ++)
    {
        /* find .dynsym section */
        os_uint8_t *shstrab;
        shstrab = (os_uint8_t *)elf_ptr + shdr[ehdr->e_shstrndx].sh_offset;
        if (strcmp((const char *)(shstrab + shdr[index].sh_name), ELF_DYNSYM) == 0)
        {
            break;
        }
    }

    /* found .dynsym section */
    if (index != ehdr->e_shnum)
    {
        int i, count = 0;
        Elf32_Sym  *symtab = OS_NULL;
        os_uint8_t *strtab = OS_NULL;

        symtab = (Elf32_Sym *)((os_uint8_t *)elf_ptr + shdr[index].sh_offset);
        strtab = (os_uint8_t *)elf_ptr + shdr[shdr[index].sh_link].sh_offset;

        for (i = 0; i < shdr[index].sh_size / sizeof(Elf32_Sym); i++)
        {
            if ((ELF_ST_BIND(symtab[i].st_info) == STB_GLOBAL)
                && (ELF_ST_TYPE(symtab[i].st_info) == STT_FUNC))
                {
                    count ++;
                }
        }

        module->symtab = (struct os_module_symtab *)os_malloc
                          (count * sizeof(struct os_module_symtab));
        module->nsym = count;
        for (i = 0, count = 0; i < shdr[index].sh_size / sizeof(Elf32_Sym); i++)
        {
            os_size_t length;

            if ((ELF_ST_BIND(symtab[i].st_info) != STB_GLOBAL)
                || (ELF_ST_TYPE(symtab[i].st_info) != STT_FUNC))
            {
                continue;
            }

            length = strlen((const char *)(strtab + symtab[i].st_name)) + 1;

            module->symtab[count].addr =
                (void *)((char *)module->mem_space + symtab[i].st_value - module->vstart_addr);
            module->symtab[count].name = os_malloc(length);
            memset((void *)module->symtab[count].name, 0, length);
            memcpy((void *)module->symtab[count].name,
                   strtab + symtab[i].st_name,
                   length);
            count ++;
        }
    }

    return OS_EOK;
}

/**
 ***********************************************************************************************************************
 * @brief           This function load ELF relocated object
 *
 * @param[out]      module          Pointer to kernel module
 * @param[in]       elf_ptr         Pointer to ELF file.
 *
 * @return          On success, return OS_EOK; on error, return OS_ERROR.
 * @retval          OS_EOK          Load ELF relocated successfully.
 * @retval          OS_ERROR        Load ELF relocated error.
 ***********************************************************************************************************************
 */
os_err_t elf_load_relocated_object(struct os_module* module, void *elf_ptr)
{
    os_uint32_t index;
    os_uint32_t rodata_addr;
    os_uint32_t bss_addr;
    os_uint32_t data_addr;
    os_uint32_t module_addr;
    os_uint32_t module_size;
    os_uint8_t *ptr;
    os_uint8_t *strtab;
    os_uint8_t *shstrab;


    module_addr = 0;
    module_size = 0;
    /* get the ELF image size */
    for (index = 0; index < ehdr->e_shnum; index ++)
    {
        /* text */
        if (IS_PROG(shdr[index]) && IS_AX(shdr[index]))
        {
            module_size += shdr[index].sh_size;
            module_addr = shdr[index].sh_addr;
        }
        /* rodata */
        if (IS_PROG(shdr[index]) && IS_ALLOC(shdr[index]))
        {
            module_size += shdr[index].sh_size;
        }
        /* data */
        if (IS_PROG(shdr[index]) && IS_AW(shdr[index]))
        {
            module_size += shdr[index].sh_size;
        }
        /* bss */
        if (IS_NOPROG(shdr[index]) && IS_AW(shdr[index]))
        {
            module_size += shdr[index].sh_size;
        }
    }

    /* no text, data and bss on image */
    if (module_size == 0) 
    {
        return OS_ERROR;
    }

    module->vstart_addr = 0;
    module->nref = 0;

    /* allocate module space */
    module->mem_space = os_malloc(module_size);
    if (module->mem_space == OS_NULL)
    {
        LOG_EXT_E("allocate space failed.\n");
        return OS_ERROR;
    }
    module->mem_size = module_size;

    /* zero all space */
    ptr = module->mem_space;
    memset(ptr, 0, module_size);


    rodata_addr = 0;
    bss_addr = 0;
    data_addr = 0;
    /* load text and data section */
    for (index = 0; index < ehdr->e_shnum; index ++)
    {
        /* load text section */
        if (IS_PROG(shdr[index]) && IS_AX(shdr[index]))
        {
            memcpy(ptr,
                      (os_uint8_t *)ehdr + shdr[index].sh_offset,
                      shdr[index].sh_size);
            LOG_EXT_D("load text 0x%x, size %d", ptr, shdr[index].sh_size);
            ptr += shdr[index].sh_size;
        }

        /* load rodata section */
        if (IS_PROG(shdr[index]) && IS_ALLOC(shdr[index]))
        {
            memcpy(ptr,
                      (os_uint8_t *)ehdr + shdr[index].sh_offset,
                      shdr[index].sh_size);
            rodata_addr = (os_uint32_t)ptr;
            LOG_EXT_D("load rodata 0x%x, size %d, rodata 0x%x", ptr, 
                shdr[index].sh_size, *(os_uint32_t *)data_addr);
            ptr += shdr[index].sh_size;
        }

        /* load data section */
        if (IS_PROG(shdr[index]) && IS_AW(shdr[index]))
        {
            memcpy(ptr,
                      (os_uint8_t *)ehdr + shdr[index].sh_offset,
                      shdr[index].sh_size);
            data_addr = (os_uint32_t)ptr;
            LOG_EXT_D("load data 0x%x, size %d, data 0x%x", ptr, 
                shdr[index].sh_size, *(os_uint32_t *)data_addr);
            ptr += shdr[index].sh_size;
        }

        /* load bss section */
        if (IS_NOPROG(shdr[index]) && IS_AW(shdr[index]))
        {
            memset(ptr, 0, shdr[index].sh_size);
            bss_addr = (os_uint32_t)ptr;
            LOG_EXT_D("load bss 0x%x, size %d", ptr, shdr[index].sh_size);
        }
    }

    /* set module entry */
    module->entry_addr = (os_module_entry_func_t)((os_uint8_t *)module->mem_space + ehdr->e_entry - 
                            module_addr);

    /* handle relocation section */
    for (index = 0; index < ehdr->e_shnum; index ++)
    {
        os_uint32_t i, nr_reloc;
        Elf32_Sym *symtab;
        Elf32_Rel *rel;

        if (!IS_REL(shdr[index]))
        {
            continue;
        }


        /* get relocate item */
        rel = (Elf32_Rel *)((os_uint8_t *)elf_ptr + shdr[index].sh_offset);

        /* locate .dynsym and .dynstr */
        symtab   = (Elf32_Sym *)((os_uint8_t *)elf_ptr +
                                 shdr[shdr[index].sh_link].sh_offset);
        strtab   = (os_uint8_t *)elf_ptr +
                   shdr[shdr[shdr[index].sh_link].sh_link].sh_offset;
        shstrab  = (os_uint8_t *)elf_ptr +
                   shdr[ehdr->e_shstrndx].sh_offset;
        nr_reloc = (os_uint32_t)(shdr[index].sh_size / sizeof(Elf32_Rel));

        /* relocate every items */
        for (i = 0; i < nr_reloc; i ++)
        {
            Elf32_Addr addr;
            Elf32_Sym *sym;

            sym = &symtab[ELF32_R_SYM(rel->r_info)];
            addr = 0;
            LOG_EXT_D("relocate symbol: %s", strtab + sym->st_name);

            if (sym->st_shndx != STN_UNDEF)
            {
                
                if ((ELF_ST_TYPE(sym->st_info) == STT_SECTION)
                    || (ELF_ST_TYPE(sym->st_info) == STT_OBJECT))
                {
                    if (strncmp((const char *)(shstrab +
                                                  shdr[sym->st_shndx].sh_name), ELF_RODATA, 8) == 0)
                    {
                        /* relocate rodata section */
                        LOG_EXT_D("rodata");
                        addr = (Elf32_Addr)(rodata_addr + sym->st_value);
                    }
                    else if (strncmp((const char *)
                                        (shstrab + shdr[sym->st_shndx].sh_name), ELF_BSS, 5) == 0)
                    {
                        /* relocate bss section */
                        LOG_EXT_D("bss");
                        addr = (Elf32_Addr)bss_addr + sym->st_value;
                    }
                    else if (strncmp((const char *)(shstrab + shdr[sym->st_shndx].sh_name),
                                        ELF_DATA, 6) == 0)
                    {
                        /* relocate data section */
                        LOG_EXT_D("data");
                        addr = (Elf32_Addr)data_addr + sym->st_value;
                    }

                    if (addr != 0) elf_arch_relocate(module, rel, addr);
                }
                else if (ELF_ST_TYPE(sym->st_info) == STT_FUNC)
                {
                    addr = (Elf32_Addr)((os_uint8_t *) module->mem_space - module_addr + sym->st_value);

                    /* relocate function */
                    elf_arch_relocate(module, rel, addr);
                }
            }
            else if (ELF_ST_TYPE(sym->st_info) == STT_FUNC)
            {
                addr = (Elf32_Addr)((os_uint8_t *) module->mem_space - module_addr + sym->st_value);
                /* relocate function */
                elf_arch_relocate(module,rel, addr);
            }
            else
            {
                if (ELF32_R_TYPE(rel->r_info) != R_ARM_V4BX)
                {
                    LOG_EXT_D("relocate symbol: %s", strtab + sym->st_name);

                    /* need to resolve symbol in kernel symbol table */
                    addr = os_module_symbol_find((const char *)(strtab + sym->st_name));
                    if (addr != (Elf32_Addr)OS_NULL)
                    {
                        elf_arch_relocate(module, rel, addr);
                        LOG_EXT_D("symbol addr 0x%x", addr);
                    }
                    else
                    {
                        LOG_EXT_E("ELF: can't find %s in kernel symbol table",strtab + sym->st_name);
                    }
                }
                else
                {
                    addr = (Elf32_Addr)((os_uint8_t *) module->mem_space - module_addr + sym->st_value);
                    elf_arch_relocate(module, rel, addr);
                }
            }

            rel ++;
        }
    }

    return OS_EOK;
}

/**
 ***********************************************************************************************************************
 * @brief           This function load ELF file(shared object or relocated object)
 *
 * @param[in]       filename         Pointer to ELF filename.
 *
 * @return          On success, return module descriptor; on error, OS_NULL is returned.
 * @retval          Not OS_NULL      Return module descriptor.
 * @retval          OS_NULL          Load ELF file error.
 ***********************************************************************************************************************
 */
struct os_module* elf_object_load(const char* filename)
{
    int fd, length = 0;
    os_err_t ret = OS_EOK;
    os_uint8_t *elf_ptr = OS_NULL;
    struct os_module *module = OS_NULL;

    fd = open(filename, O_RDONLY, 0);
    if (fd >= 0)
    {
        length = lseek(fd, 0, SEEK_END);
        lseek(fd, 0, SEEK_SET);
        if (length == 0)
        {
            goto __exit;
        }

        elf_ptr = (uint8_t*)malloc (length);
        if (!elf_ptr)
        {
            goto __exit;
        }

        if (read(fd, elf_ptr, length) != length)
        {
            goto __exit;
        }
        
        /* close file and release fd */
        close(fd);
        fd = -1;
    }
    else
    {
        goto __exit;
    }

    /* check ELF header */
    if (memcmp(ehdr->e_ident, RTMMAG, SELFMAG) != 0
        && memcmp(ehdr->e_ident, ELFMAG, SELFMAG) != 0)
    {
        LOG_EXT_E("ELF magic error\n");
        goto __exit;
    }

    /* check ELF class */
    if (ehdr->e_ident[EI_CLASS] != ELFCLASS32)
    {
        LOG_EXT_E("ELF class error\n");
        goto __exit;
    }
  
    module = os_module_create();
  
    if (!module)
    {
        goto __exit;
    }

    /* set the name of module */
    os_module_set_name(module, filename);

    LOG_EXT_D("elf_load: %.*s", OS_NAME_MAX, module->parent.name);

    if (ehdr->e_type == ET_REL)
    {
        ret = elf_load_relocated_object(module, elf_ptr);
    }
    else if (ehdr->e_type == ET_DYN)
    {
        ret = elf_load_shared_object(module, elf_ptr);
    }
    else
    {
        LOG_EXT_E("unsupported elf type\n");
        goto __exit;
    }

    /* check return value */
    if (ret != OS_EOK)
    {
        goto __exit;
    }

    /* release module data */
    free(elf_ptr);

    /* increase module reference count */
    module->nref ++;

    /* deal with cache */
#ifdef OS_USING_CACHE
    os_hw_cpu_dcache_ops(OS_HW_CACHE_FLUSH, module->mem_space, module->mem_size);
    os_hw_cpu_icache_ops(OS_HW_CACHE_INVALIDATE, module->mem_space, module->mem_size);
#endif
    
    /* set module initialization and cleanup function */
    module->init_func = (os_module_init_func_t)dlsym(module, "module_init");
    module->cleanup_func = (os_module_cleanup_func_t)dlsym(module, "module_cleanup");    

    module->stat = OS_MODULE_STAT_INIT;
    /* do module initialization */
    if (module->init_func)
    {
        module->init_func(module);
    }

    return module;

__exit:
    if (fd >= 0)
    {
        close(fd);
    }
    if (elf_ptr)
    {
        free(elf_ptr);
    }
    if (module)
    {
        os_module_destroy(module);
    }

    return OS_NULL;
}

#define ELF_ARG_MAX    8
static int _elf_split_arg(char *cmd, os_size_t length, char *argv[])
{
    int argc = 0;
    char *ptr = cmd;

    while ((ptr - cmd) < length)
    {
        /* strip bank and tab */
        while ((*ptr == ' ' || *ptr == '\t') && (ptr - cmd) < length)
        {
            *ptr++ = '\0';
        }
        /* check whether it's the end of line */
        if ((ptr - cmd) >= length)
        {
            break;
        }

        /* handle string with quote */
        if (*ptr == '"')
        {
            argv[argc++] = ++ptr;

            /* skip this string */
            while (*ptr != '"' && (ptr - cmd) < length)
            {
                if (*ptr++ == '\\')
                {
                    ptr++;
                }
            }

            if ((ptr - cmd) >= length)
            {
                break;
            } 

            /* skip '"' */
            *ptr++ = '\0';
        }
        else
        {
            argv[argc++] = ptr;
            while ((*ptr != ' ' && *ptr != '\t') && (ptr - cmd) < length)
            {
                ptr ++;
            }        
        }

        if (argc >= ELF_ARG_MAX)
        {
            break;
        }
    }

    return argc;
}


static void _elf_task_entry(void* parameter)
{
    int argc = 0;
    char *argv[ELF_ARG_MAX];

    struct os_module *module = (struct os_module*)parameter;

    if (module == OS_NULL || module->cmd_line == OS_NULL)
    {
        /* malloc for module_cmd_line failed. */
        return;
    }

    if (module->cmd_line)
    {
        memset(argv, 0x00, sizeof(argv));
        argc = _elf_split_arg((char *)module->cmd_line, strlen(module->cmd_line), argv);
        if (argc == 0)
        {
            goto __exit;
        }
    }
    
    /* set status of module */
    module->stat = OS_MODULE_STAT_RUNNING;

    LOG_EXT_D("run main entry: 0x%p with %s",
              module->entry_addr,
              module->cmd_line);

    if (module->entry_addr)
    {
        module->entry_addr(argc, argv);
    }

__exit:
    os_module_stop_child_task();

    return ;
}
/**
 ***********************************************************************************************************************
 * @brief           This function execute ELF (shared object).
 *
 * @details         Simulate a process using a runnable shared library;ELF shared object run in the kernel as modules.
 *                  At the end of a shared object execution , garbage collection is uniformly placed idle task
 *                   (defunct task)
 * @param[out]      module          Pointer to kernel module
 * @param[in]       elf_ptr         Pointer to elf file.
 *
 * @return          On success, return OS_EOK; on error, return OS_ERROR.
 * @retval          OS_EOK          Execute shared object successfully.
 * @retval          OS_NULL         Execute shared object error.
 ***********************************************************************************************************************
 */
os_err_t elf_shared_object_exec(const char* pgname, const char* cmd, int cmd_size)
{
    struct os_module *module;
    os_size_t len;
    
    module = elf_object_load(pgname);
    
    if (module)
    {
        if (module->entry_addr)
        {
            /* exec this module */
            os_task_t *tid;
            
            len = strlen (cmd) + 1;
            module->cmd_line = os_malloc(len);
            if (module->cmd_line)
            {
                memcpy(module->cmd_line, cmd, len);
            }
            
            /* check stack size and priority */
            if (module->priority > OS_TASK_PRIORITY_MAX)
            {
                module->priority = OS_TASK_PRIORITY_MAX - 1;
            }
            if (module->stack_size < 2048 || module->stack_size > (1024 * 32)) 
            {
                module->stack_size = 2048;
            }

            tid = os_task_create(module->parent.name,
                                 _elf_task_entry, 
                                 (void*)module, 
                                 module->stack_size,
                                 module->priority,
                                 10);
            if (tid)
            {
                tid->module_id = module;
                module->main_task = tid;

                os_task_startup(tid);
            }
            else
            {
                /* destory kernel module */
                os_module_destroy(module);
                return OS_ERROR;
            }
        }
    }

    return OS_EOK;
}

