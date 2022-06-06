/* ** por compatibilidad se omiten tildes **
================================================================================
 TALLER System Programming - ORGANIZACION DE COMPUTADOR II - FCEN
================================================================================

  Definiciones globales del sistema.
*/

#ifndef __DEFINES_H__
#define __DEFINES_H__

/* Misc */
/* -------------------------------------------------------------------------- */
// Y Filas
#define SIZE_N 40
#define ROWS   SIZE_N

// X Columnas
#define SIZE_M 80
#define COLS   SIZE_M

/* Indices en la gdt */
/* -------------------------------------------------------------------------- */
#define GDT_COUNT         35

#define GDT_IDX_NULL_DESC 0
#define GDT_IDX_CODE_0 1
#define GDT_IDX_CODE_3 2
#define GDT_IDX_DATA_0 3
#define GDT_IDX_DATA_3 4
#define GDT_IDX_VIDEO  5

/* Offsets en la gdt */
/* -------------------------------------------------------------------------- */
#define GDT_OFF_NULL_DESC (GDT_IDX_NULL_DESC << 3)
#define GDT_OFF_VIDEO  (GDT_IDX_VIDEO << 3)

/* COMPLETAR - Valores para los selectores de segmento de la GDT 
 * Definirlos a partir de los índices de la GDT, definidos más arriba 
 * Hint: usar operadores "<<" y "|" (shift y or) */

#define GDT_CODE_0_SEL (1 << 3)  
#define GDT_DATA_0_SEL 3 << 3
#define GDT_CODE_3_SEL (2 << 3) | 3
#define GDT_DATA_3_SEL (4 << 3) | 3


// Macros para trabajar con segmentos de la GDT.

// SEGM_LIMIT_4KIB es el limite de segmento visto como bloques de 4KIB
// principio del ultimo bloque direccionable.
#define GDT_LIMIT_4KIB(X)  (((X) / 4096) - 1)
#define GDT_LIMIT_BYTES(X) ((X)-1)

#define GDT_LIMIT_LOW(limit)  (uint16_t)(((uint32_t)(limit)) & 0x0000FFFF)
#define GDT_LIMIT_HIGH(limit) (uint8_t)((((uint32_t)(limit)) >> 16) & 0x0F)

#define GDT_BASE_LOW(base)  (uint16_t)(((uint32_t)(base)) & 0x0000FFFF)
#define GDT_BASE_MID(base)  (uint16_t)((((uint32_t)(base)) >> 16) & 0xFF)
#define GDT_BASE_HIGH(base)  (uint16_t)((((uint32_t)(base)) >> 24) & 0xFF)
/* COMPLETAR - Valores de atributos */ 
#define DESC_CODE_DATA 1
#define DESC_SYSTEM 0
#define DESC_TYPE_EXECUTE_READ 10
#define DESC_TYPE_READ_WRITE   2
#define DESC_LIMIT 0x330FF

/* COMPLETAR - Tamaños de segmentos */ 
#define FLAT_SEGM_SIZE  856686592
#define VIDEO_SEGM_SIZE 800


/* Direcciones de memoria */
/* -------------------------------------------------------------------------- */

// direccion fisica de comienzo del bootsector (copiado)
#define BOOTSECTOR 0x00001000
// direccion fisica de comienzo del kernel
#define KERNEL 0x00001200
// direccion fisica del buffer de video
#define VIDEO 0x000B8000

/* MMU */
/* -------------------------------------------------------------------------- */
/* Definan:
VIRT_PAGE_OFFSET(X) devuelve el offset dentro de la página, donde X es una dirección virtual
VIRT_PAGE_TABLE(X)  devuelve la page table entry correspondiente, donde X es una dirección virtual
VIRT_PAGE_DIR(X)    devuelve el page directory entry, donde X es una dirección virtual

una direccion virtual es de 32bits, donde:

  --------------------------------------
  |                                    |
  --------------------------------------
  |  dirIdx  |  tableIdx  |   offset   |
       10         10            12


  La dirVirt es esta:
  -----------------------
  |  10  |  10  |   12  |
  -----------------------

  *Si quiero el offset: 

  -----------------------
  |  10  |  10  |   12  |
  -----------------------
&  00000..00000 |  FFF 
:   
                ---------
                |   12  |
                ---------    

  *Si quiero el tableIdx (los 10 del medio):
  1) >> 12 para quedarme con los 20bits mas signif

    -----------------------
    |  10  |  10  |   12  |
    -----------------------
  >> 12:
    -----------------------
    |       |  10  |  10  |
    -----------------------
  2) quedarme con los 10bits menos signif

    -----------------------
    |       |  10  |  10  |
    -----------------------
  & 000 ...  000   |  3FF  :      (3FF son diez 1's)

                  ---------
                  |   10  |
                  ---------  

  *Si quiero el dirIdx (los 10bits mas signif):

    -----------------------
    |  10  |  10  |   12  |
    -----------------------
  >> 22:
    -----------------------
    |              |  10  |
    -----------------------

CR3_TO_PAGE_DIR(X)  devuelve el page directory, donde X es el contenido del registro CR3

El Registro CR3 tiene esta pinta:

  -----------------------------------------------------------------------------
  |                                      |                                    |
  -----------------------------------------------------------------------------
  |             dirBaseDirect            |  ignorados | pcd | pwt | ignorados |
                    20                              12(en total)
                                                    (todos en 0)

  Queremos quedarnos con la direcc del page directory, osea los 20bits mas signif
  Asique le hacemos: & 0xFFFFF000 


MMU_ENTRY_PADDR(X)  devuelve la dirección física de la base de un page frame o de un page table, donde X es el campo de 20 bits en una PTE o PDE

*/

#define VIRT_PAGE_OFFSET(X) (uint16_t)(((uint32_t)(X)) & 0x00000FFF)          
#define VIRT_PAGE_TABLE(X)  (uint16_t)(((uint32_t)(X) >> 12 ) & 0x000003FF)   
#define VIRT_PAGE_DIR(X)    (uint16_t)(((uint32_t)(X)) >> 22)
#define CR3_TO_PAGE_DIR(X)  ((uint32_t)(X) & 0xFFFFF000)
#define MMU_ENTRY_PADDR(X)  (((uint32_t)(X)) & 0xFFFFF000)


/*
    1 : 0000 0001
    
    1 << 0 = 0000 0001

    1 << 1 = 0000 0010

    1 << 2 = 0000 0100

*/
#define MMU_P (1 << 0)  // flag Presente            (el bit 0 (el menos signif))
#define MMU_W (1 << 1)  // flag Lectura Y Escritura (el bit 1)
#define MMU_U (1 << 2)  // flag Supervisor          (el bit 2)

#define BASE_VALUE(X)  ((uint32_t)(X) >> 12)

#define PAGE_SIZE 4096

// direccion virtual del codigo
#define TASK_CODE_VIRTUAL 0x08000000
#define TASK_CODE_PAGES   2
#define TASK_STACK_BASE   0x08003000

/* Direcciones fisicas de directorios y tablas de paginas del KERNEL */
/* -------------------------------------------------------------------------- */
#define KERNEL_PAGE_DIR     0x00025000
#define KERNEL_PAGE_TABLE_0 0x00026000
#define KERNEL_STACK        0x00025000



#endif //  __DEFINES_H__

