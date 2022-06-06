/* ** por compatibilidad se omiten tildes **
================================================================================
 TRABAJO PRACTICO 3 - System Programming - ORGANIZACION DE COMPUTADOR II - FCEN
================================================================================

  Declaracion de funciones del manejador de memoria
*/

#ifndef __MMU_H__
#define __MMU_H__

#include "types.h"

// Page Directory Entry
typedef struct pd_entry_t {
  uint32_t attrs : 12;        // 12bits de atributos (los 12 menos signif)
  uint32_t pt : 20;           // Dir Base de la Page Table a la que apunta este entry
} __attribute__((packed)) pd_entry_t;

// Page Table Entry
typedef struct pt_entry_t {
  uint32_t attrs : 12;        // 12bits de atributos (los 12 menos signif)
  uint32_t page : 20;         // Dir Base de la Pagina apuntada por este entry
} __attribute__((packed)) pt_entry_t;

void mmu_init(void);


paddr_t mmu_next_free_kernel_page(void);

paddr_t mmu_next_free_user_page(void);

void mmu_map_page(uint32_t cr3, vaddr_t virt, paddr_t phy, uint32_t attrs);

paddr_t mmu_unmap_page(uint32_t cr3, vaddr_t virt);

void copy_page(paddr_t dst_addr, paddr_t src_addr);

paddr_t mmu_init_kernel_dir(void);

paddr_t mmu_init_task_dir(paddr_t phy_start);

#endif //  __MMU_H__
