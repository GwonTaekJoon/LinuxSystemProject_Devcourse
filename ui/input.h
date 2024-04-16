#ifndef _INPUT_H
#define _INPUT_H

int create_input();
#include <stdint.h>




/* struct for ELF file structure */

typedef struct _Elf64Hdr{

    uint8_t e_ident[16]; /* Elf magic number, other info like endian os.. etc */
    uint16_t e_type; /* file type like executable, objetct ...   */
    uint16_t e_machine; /* cpu architecture  */
    uint32_t e_version; /* ELF version - fixed 0x01 except for invalid version    */
    uint64_t e_entry; /* The virtual address to which the system first transfers control, thus starting the process. If the file has no associated entry point, this member holds zero.   */
    uint64_t e_phoff; /* Program header table file offset  */
    uint64_t e_shoff; /* section header table file offset  */
    uint32_t    e_flags;             /* Processor-specific flags */
  uint16_t    e_ehsize;            /* ELF header size in bytes */
  uint16_t    e_phentsize;         /* Program header table entry size */
  uint16_t    e_phnum;             /* Program header table entry count */
  uint16_t    e_shentsize;         /* Section header table entry size */
  uint16_t    e_shnum;             /* Section header table entry count */
  uint16_t    e_shstrndx;          /* Section header string table index */

} Elf64Hdr;
#endif /* _INPUT_H */
