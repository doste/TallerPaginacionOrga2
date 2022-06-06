/* ** por compatibilidad se omiten tildes **
================================================================================
 TRABAJO PRACTICO 3 - System Programming - ORGANIZACION DE COMPUTADOR II - FCEN
================================================================================

  Definicion de funciones del manejador de memoria
*/

#include "mmu.h"
#include "i386.h"

#include "kassert.h"

static pd_entry_t* kpd = (pd_entry_t*)KERNEL_PAGE_DIR;
static pt_entry_t* kpt = (pt_entry_t*)KERNEL_PAGE_TABLE_0;

//static const uint32_t identity_mapping_end = 0x003FFFFF;
//static const uint32_t user_memory_pool_end = 0x02FFFFFF;

static paddr_t next_free_kernel_page = 0x100000;
static paddr_t next_free_user_page = 0x400000;

/**
 * kmemset asigna el valor c a un rango de memoria interpretado 
 * como un rango de bytes de largo n que comienza en s 
 * @param s es el puntero al comienzo del rango de memoria
 * @param c es el valor a asignar en cada byte de s[0..n-1]
 * @param n es el tamaño en bytes a asignar
 * @return devuelve el puntero al rango modificado (alias de s)
*/
static inline void* kmemset(void* s, int c, size_t n) {
  uint8_t* dst = (uint8_t*)s;
  for (size_t i = 0; i < n; i++) {
    dst[i] = c;
  }
  return dst;
}

/**
 * zero_page limpia el contenido de una página que comienza en addr
 * @param addr es la dirección del comienzo de la página a limpiar
*/
static inline void zero_page(paddr_t addr) {
  kmemset((void*)addr, 0x00, PAGE_SIZE);
}


void mmu_init(void) {}


/**
 * mmu_next_free_kernel_page devuelve la dirección de la próxima página de kernel disponible
 * @return devuelve la dirección de memoria de comienzo de la próxima página libre de kernel
 */
paddr_t mmu_next_free_kernel_page(void) {
    paddr_t temp = next_free_kernel_page;
    next_free_kernel_page = next_free_kernel_page + PAGE_SIZE;
    return temp;
}

/**
 * mmu_next_free_user_page devuelve la dirección de la próxima página de usuarix disponible
 * @return devuelve la dirección de memoria de comienzo de la próxima página libre de usuarix
 */
paddr_t mmu_next_free_user_page(void) {
    paddr_t temp = next_free_user_page;
    next_free_user_page = next_free_user_page + PAGE_SIZE;
    return temp;
}

/**
 * mmu_init_kernel_dir inicializa las estructuras de paginación vinculadas al kernel y
 * realiza el identity mapping
 * @return devuelve la dirección de memoria de la página donde se encuentra el directorio 
 * de páginas usado por el kernel
 */

/*
paddr_t mmu_init_kernel_dir(void) {
    paddr_t* directorio = (paddr_t *)KERNEL_PAGE_DIR;
    paddr_t* tabla = (paddr_t *)KERNEL_PAGE_TABLE_0;

    // completamos todo con 0 inicialmente, basicamente invalidamos las 1024 entradas de ambas
    zero_page(*directorio);
    zero_page(*tabla);

    // la primer entrada del directorio quiero que apunte a la tabla de paginas del kernel
    directorio[0] = BASE_VALUE(tabla) | MMU_P | MMU_W | MMU_U;
    // me quedo con los 20bits mas signif de la dir de la tabla y le seteo los bits de Presente, Escritura y Supervisor

    // armo el IdentityMapping de los primeros 4MB
    for (int i = 0; i < 1024; i++) {
        tabla[i] = (i << 12) | MMU_P | MMU_W | MMU_U;
        // a cada entry de la tabla le seteamos su misma direcc (shifteada 12 asi nos 'salteamos' los 12bits menos signif,
        // los cuales seran los attrs que los completamos despues agregandolos a mano
    }

    return *directorio;
}
*/
//UN POCO MAS COMPLICADA AL PEDO, mas facil asi:
paddr_t mmu_init_kernel_dir(void) {
    zero_page(KERNEL_PAGE_DIR);
    zero_page(KERNEL_PAGE_TABLE_0);

    // inicializo una PageDirectoryEntry, la cual sera la primera del Directorio.
    // apuntara a la table de paginas del kernel y tendra los atributos de Presente, Escritura y Supervisor (los 3bits menos signif = 111 = 3)
    pd_entry_t d= {0x003,(KERNEL_PAGE_TABLE_0)>>12};
    
    // kpd es un puntero global a la KERNEL_PAGE_DIR
    kpd[0]= d;    // seteo la primera entry del directorio para que apunte a la tabla
    for(int i=0; i<1024; i++) {
      // para cada una de las entrys de la tabla de paginas creo una PageTableEntry seteandole los mismos atributos que antes y la direccion base
      // sera igual que el numero correspondiente a cada entry (IdentityMapping)
      pt_entry_t t= {0x003,i};
      // kpt es un puntero global a KERNEL_PAGE_TABLE_0
      // hago que la entrada i de la tabla apunte a la pt_entry_t i
      kpt[i]= t ;
    }
    return KERNEL_PAGE_DIR;
}



/**
 * mmu_map_page agrega las entradas necesarias a las estructuras de paginación de modo de que
 * la dirección virtual virt se traduzca en la dirección física phy con los atributos definidos en attrs
 * @param cr3 el contenido que se ha de cargar en un registro CR3 al realizar la traducción
 * @param virt la dirección virtual que se ha de traducir en phy
 * @param phy la dirección física que debe ser accedida (dirección de destino)
 * @param attrs los atributos a asignar en la entrada de la tabla de páginas
*/ 
/*
void mmu_map_page(uint32_t cr3, vaddr_t virt, paddr_t phy, uint32_t attrs) {

    // Queremos mapear la direccion de memoria virtual virt a la direccion de memoria fisica phy
    // todo esto en el esquema de paginacion indicado por cr3

    // Primer paso: dividir la direccion a mapear en dirIdx, tableIdx y offset
    // page directory index:
    uint16_t pd_index= VIRT_PAGE_DIR(virt);   
    // page table index:
    uint16_t pt_index= VIRT_PAGE_TABLE(virt);
    //uint16_t p_offset= VIRT_PAGE_OFFSET(virt); 

    // usamos el cr3 para calcular la direcc del Page Directory
    // puntero al page directory (el cr3 apunta al Page Directory)
    pd_entry_t *PDT= (pd_entry_t*)(CR3_TO_PAGE_DIR(cr3));


    // page directory entry . en todo el directorio de paginas, me quedo con la entry indicada por
    // pd_index, el cual esta en la dir virtual
    pd_entry_t PDE= PDT[pd_index];


    // if (PDE.Present != 1) Osea que la pagina que voy a usar como PageDirectoryEntry no esta en memoria, en ese caso pido al kernel una nueva pagina
    // y actualizo el puntero .pt a la nueva posicion
    if(!PDE.attrs&1)
    {
        paddr_t newPT = mmu_next_free_kernel_page(); // el kernel me devuelve una pagina del Area Libre de Kernel de la memoria ( direccs: 0x100000 a 0x3FFFFF)
        zero_page(newPT);                      // la inicializo en 0, osea la completo toda con ceros
        PDE.pt = newPT>>12;                    // me quedo con los 20 bits mas signif de la pagina devuelta por el kernel, los cuales indican la direccion fisica
                                               // de la pagina devuelta. esos 20bits se los seteo en el .pt del PageDirectoryEntry que estoy seteando
    }
    // ya sea que estaba presente o no, ahora tenemos una PageDirectoryEntry seteada
    
    PDE.attrs= attrs|1;                        // los atributos del PageDirectoryEntry seran los pasados por parametro pero ademas seteandole el bit Presente en 1
    PDT[pd_index] = PDE;                       // hacemos que la Entry en la PageDirectoryTable en el indice pd_index apunte al PageDirectoryEntry que acabamos de configurar
    
    // Obtenemos el PageTableEntry a partir del comienzo del PageTable y del campo tableIndex (pt_index)

    // primero armamos un puntero a la base del PageTable
    // la dir base de un PageTable esta dada por los 20bits mas signif del PageDirectoryEntry correspondiente

    //pt_entry_t* PT_base = ((pt_entry_t*)((uint32_t)PDE.pt)); 
    pt_entry_t* PT_base = ((pt_entry_t*)((uint32_t)PDE.pt<<12)); 

    // a partir del puntero a la base del PageTable, me quedo con el entry indexado por pt_index (que viene de la dir virtual)
    pt_entry_t PTE= PT_base[pt_index];
    // ahora tenemos el PageTableEntry, lo completamos con el marco de pagina que se busca mapear:
    PTE.page = phy & 0xFFFFF000 ;     // los 20bits mas signif apuntaran a la pagina fisica (los 20bits mas signif de esta)
    PTE.attrs = attrs|1;              // tambien le seteamos los attrs a la entry
    PT_base[pt_index] = PTE;          // que la entry indicada por el tableIdx, apunte a esta PageTableEntry que acabamos de configurar

    // por ultimo llamamos a tlbflush para que invalide todas las entradas de la TLB
    tlbflush();

}
*/
// ESTO DE ACA ARRIBA NO FUNCIONA . Pero tiene comentarios utiles
/*
void mmu_map_page(uint32_t cr3, vaddr_t virt, paddr_t phy, uint32_t attrs) {
    
    // page directory index
    uint16_t pd_index= VIRT_PAGE_DIR(virt);   
    // page table index
    uint16_t pt_index= VIRT_PAGE_TABLE(virt);
    //uint16_t p_offset= VIRT_PAGE_OFFSET(virt); 

    // puntero al page directory (el cr3 apunta al Page Directory)
    pd_entry_t *PDT= (pd_entry_t*)(CR3_TO_PAGE_DIR(cr3));
    // page directory entry . en todo el directorio de paginas, me quedo con la entry indicada por
    // pd_index, el cual esta en la dir virtual
    pd_entry_t PDE= PDT[pd_index];
    // puntero a una Page Table
    pt_entry_t *PT;
    // if (PDE.Present != 1) Osea que la pagina que voy a usar como PageDirectoryEntry no esta en memoria, en ese caso pido al kernel una nueva pagina
    if(!PDE.attrs&1)
    {
        paddr_t newPageTable = mmu_next_free_kernel_page(); // el kernel me devuelve una pagina del Area Libre de Kernel de la memoria ( direccs: 0x100000 a 0x3FFFFF)
        zero_page(newPageTable);                      // la inicializo en 0, osea la completo toda con ceros
        PDE.pt = newPageTable>>12;                    // me quedo con los 20 bits mas signif de la pagina devuelta por el kernel, los cuales indican la direccion fisica
                                               // de la pagina devuelta. esos 20bits se los seteo en el .pt del PageDirectoryEntry que estoy seteando
    }


    PDE.attrs= attrs|1;                        // los atributos del PageDirectoryEntry seran los pasados por parametro pero ademas seteandole el bit Presente en 1
    PDT[pd_index] = PDE;                       // hacemos que la Entry en la PageDirectoryTable en el indice pd_index apunte al PageDirectoryEntry que acabamos de configurar
    PT= ((pt_entry_t*)((uint32_t)PDE.pt<<12)); // el puntero a PageTable apunte a el .pt del PageDirectoryEntry que 
    pt_entry_t PTE= PT[pt_index];
    PTE.page = phy >> 12 ; 
    PTE.attrs = attrs|1;
    PT[pt_index] = PTE;
    tlbflush();
} 
*/
// ESTE DE ACA ARRIBA FUNCIONA . pero esta medio confuso, lo reescribo:
/*
    Una diferencia importante es que ahora voy a usar punteros. 
    Por ejemplo, antes tenia que hacer

        pd_entry_t PDE= PDT[pd_index];

    configurar PDE y despues volver a hacer:

        PDT[pd_index] = PDE;  
    
    lo mismo con pt_entry_t PTE= PT[pt_index]; y PT[pt_index] = PTE;

    Mejor usar punteros y listo

*/
void mmu_map_page(uint32_t cr3, vaddr_t virt, paddr_t phy, uint32_t attrs) {
    
    // page directory index
    uint16_t directoryIdx = VIRT_PAGE_DIR(virt);   
    // page table index
    uint16_t tableIdx = VIRT_PAGE_TABLE(virt);

    pd_entry_t *pageDirectoryTable = (pd_entry_t*)(CR3_TO_PAGE_DIR(cr3));
    
    pd_entry_t* pageDirectoryEntry = &pageDirectoryTable[directoryIdx];
    

    // Si el bit Presente de la PDE es 0:
    if ( !(pageDirectoryEntry->attrs & 1) )
    {
        paddr_t* newPageTable = (paddr_t *)mmu_next_free_kernel_page();     // Pido una nueva página para la PageTable
        zero_page(*newPageTable);                                           // la cargo con 0s
        pageDirectoryEntry->pt = (uint32_t)newPageTable>>12;                // me quedo con los 20bits mas signif
        // hago que ahora esta entry apunte a la nueva PageTable

        // pageDirectoryEntry no estaba Presente, la marco como tal:
        pageDirectoryEntry->attrs = attrs|1;                  
                                              
    }

    // Obtengo la PageTable de la PDE
	  pt_entry_t *pageTable = (pt_entry_t *)(pageDirectoryEntry->pt << 12);
    // tengo que completar una pt_entry_t (32bits) . tengo la pageDirectoryEntry->pt que es de 20bits, bueno tengo que 'correr' esos 
    // 20bits a la izquierda para que me quede lugar para los attrs
    // (sino hiciera el << 12 , los 20bits del ->pt me querian en los 20bits menos signif del pt_entry_t, y quiero que esten en los MAS signif)


    // Usando el puntero a la PageTable y el campo tableIdx, obtengo la PageTableEntry (PTE)
	  // Completar los atributos (seteandole ademas el Presente en 1) y la direcc base fisica en la PTE
    pageTable[tableIdx].attrs = attrs|1;

    pageTable[tableIdx].page = phy >> 12;
    
    // Optimización: comparar cr3 parámetro con cr3 real para evitar hacer siempre el flush
    if (rcr3() != cr3) {
      tlbflush();
    }
} 



/**
 * mmu_unmap_page elimina la entrada vinculada a la dirección virt en la tabla de páginas correspondiente
 * @param virt la dirección virtual que se ha de desvincular
 * @return la dirección física de la página desvinculada
*/
/*
paddr_t mmu_unmap_page(uint32_t cr3, vaddr_t virt) {
    // Obtiene la PageTableEntry correspondiente a la direcc virtual virt y la invalida, efectivamente desmapeando la direccion

    // page directory index
    uint16_t pd_index= VIRT_PAGE_DIR(virt);   
    // page table index
    uint16_t pt_index= VIRT_PAGE_TABLE(virt);
    uint16_t p_offset= VIRT_PAGE_OFFSET(virt); 


    pd_entry_t *PDT= (pd_entry_t*)(CR3_TO_PAGE_DIR(cr3));
    // page directory entry . en todo el directorio de paginas, me quedo con la entry indicada por
    // pd_index, el cual esta en la dir virtual
    pd_entry_t PDE= PDT[pd_index];
    
    paddr_t phy = 0;
    
    // Solo efectuo el desmapeo si la pagina a desmapear esta en memoria, osea esta efectivamente mapeada, osea el bit Presente en 1
    if(PDE.attrs&1) {
      
      //pt_entry_t *PT = ((pt_entry_t*)((uint32_t)PDE.pt<<12));
      pt_entry_t *PT = ((pt_entry_t*)((uint32_t)PDE.pt));
      pt_entry_t PTE= PT[pt_index];
      // a la PageTableEntry correspondiente a la direcc a desmapear, le seteo el bit Presente en 0:
      PTE.attrs= PTE.attrs & 0xffe; // (0xffe son todos 1's menos el bit menos signif, el cual seria un 0)

      // obtengo la direcc fisica mapeada a la PageTableEntry
      // PTE.page me indica los 20bits mas signif osea la base del marco de la pagina
      // a eso le debo sumar el offset (que viene de la direcc virtual)
      //phy = (PTE.page)<<12 | p_offset;
      phy = PTE.page | p_offset;

      // limpio los 20bits mas signif de la PageTableEntry para que ahora su pagina no apunte a nada
      PTE.page = 0;

      // efectuo todos estos cambios que hice, de modo que afecten a la PageTable, en particular a la entry indicada por el indice pt_index
      PT[pt_index] = PTE;

      tlbflush();
    }
    
    return phy;
}
*/

paddr_t mmu_unmap_page(uint32_t cr3, vaddr_t virt) {
    // Obtiene la PageTableEntry correspondiente a la direcc virtual virt y la invalida, efectivamente desmapeando la direccion

    // page directory index
    uint16_t directoryIdx = VIRT_PAGE_DIR(virt);   
    // page table index
    uint16_t tableIdx = VIRT_PAGE_TABLE(virt);

    uint16_t page_offset= VIRT_PAGE_OFFSET(virt); 

    pd_entry_t *pageDirectoryTable = (pd_entry_t*)(CR3_TO_PAGE_DIR(cr3));
    
    // como no lo voy a modificar no lo declaro como puntero
    pd_entry_t pageDirectoryEntry = pageDirectoryTable[directoryIdx];

    // direcc fisica a retornar
    paddr_t phy = 0;

    // Si el bit de present de la PDE es 1
    if (pageDirectoryEntry.attrs & 1) {
      // Obtener la PageTable
      pd_entry_t* pageTable = (pd_entry_t *)(pageDirectoryEntry.pt << 12);
      // Le seteo el attr de Presente en 0 a la PageTableEntry
      pageTable[tableIdx].attrs = pageTable[tableIdx].attrs & 0xffe;

      // obtengo la direcc fisica a partir del .pt del entry
      // para eso me paro en la base del marco de pagina ( pageTable[tableIdx].pt ) y le 'sumo' el offset que viene de la direcc virtual
      phy = pageTable[tableIdx].pt | page_offset;

      pageTable[tableIdx] = (pd_entry_t){0}; //Alcanza con poner presente en 0

      tlbflush();
    }

    return phy;
}

#define DST_VIRT_PAGE 0xA00000
#define SRC_VIRT_PAGE 0xB00000

/**
 * copy_page copia el contenido de la página física localizada en la dirección src_addr a la página física ubicada en dst_addr
 * @param dst_addr la dirección a cuya página queremos copiar el contenido
 * @param src_addr la dirección de la página cuyo contenido queremos copiar
 * 
 * Esta función mapea ambas páginas a las direcciones SRC_VIRT_PAGE y DST_VIRT_PAGE, respectivamente, realiza
 * la copia y luego desmapea las páginas. Usar la función rcr3 definida en i386.h para obtener el cr3 actual
*/ 
void copy_page(paddr_t dst_addr, paddr_t src_addr) {

    //mmu_map_page(uint32_t cr3, vaddr_t virt, paddr_t phy, uint32_t attrs) 
    uint32_t cr3 = rcr3();
    mmu_map_page(cr3, DST_VIRT_PAGE, dst_addr, 3);
    mmu_map_page(cr3, SRC_VIRT_PAGE, src_addr, 3);

    uint32_t* dst_virt_page =  (uint32_t *)DST_VIRT_PAGE;
    uint32_t* src_virt_page =  (uint32_t *)SRC_VIRT_PAGE;

    for (int i = 0; i < 1024; i++) {
        dst_virt_page[i] = src_virt_page[i];
    }

    mmu_unmap_page(cr3, DST_VIRT_PAGE);
    mmu_unmap_page(cr3, SRC_VIRT_PAGE);

}


 /**
 * mmu_init_task_dir inicializa las estructuras de paginación vinculadas a una tarea cuyo código se encuentra en la dirección phy_start
 * @pararm phy_start es la dirección donde comienzan las dos páginas de código de la tarea asociada a esta llamada
 * @return el contenido que se ha de cargar en un registro CR3 para la tarea asociada a esta llamada
*/ 
paddr_t mmu_init_task_dir(paddr_t phy_start) {

    // tengo que inicializar las estructuras de paginación vinculadas a una tarea, esto es armar un directorio y una pagetable vinculado a el

    // pido 2 paginas al kernel. una para el directorio y otra para la tabla
    paddr_t directoryStart = mmu_next_free_kernel_page() ;
    paddr_t tablaStart = mmu_next_free_kernel_page() ;

    // 'limpio' toda la pagina del directorio
    zero_page(directoryStart);

    // dos punteros para poder ahora modificar tanto al directorio como a la pagetable
    pd_entry_t* pagedirectory= (pd_entry_t*)directoryStart;
    pt_entry_t* pagetable= (pt_entry_t*)tablaStart;

    // me armo un PageDirectoryEntry, con attrs de Lectura/Escritura, Supervisor y Presente
    // la PageTable a la que apunte sera la pagetable que recien pedimos
    pd_entry_t pageDirectoryEntry = {.attrs = 0x003, .pt = (tablaStart)>>12};

    // hago que la primera entrada del directorio sea la Entry recien configurada
    pagedirectory[0] = pageDirectoryEntry;
    // osea que la Entry 0 del directorio apunte a la pagetable que pedimos

    // Identity Mapping para los primeros 4MB
    for (int i = 0; i<1024; i++)
    {
      pt_entry_t pageTableEntry = { .attrs = 0x003, .page = i};
      pagetable[i] = pageTableEntry;
      // que cada Entry del pagetable apunte a una pagina con la misma direcc
    }

    uint32_t cr3 = directoryStart;

    // phy_start es la dirección donde comienzan las dos páginas de código de la tarea asociada a esta llamada
    //"La rutina debe mapear las páginas de código como solo lectura, a partir de la dirección virtual 0x08000000"
    mmu_map_page(cr3, TASK_CODE_VIRTUAL, phy_start, 5);                         // attrs = 5 = 0101 (Supervisor y Presente en 1, solo Lectura)
    mmu_map_page(cr3, TASK_CODE_VIRTUAL + PAGE_SIZE, phy_start+ PAGE_SIZE, 5);

    // "y el stack como lectura-escritura con base en 0x08003000"
    // el stack crece para abajo, entonces su base en verdad seria su tope
    // asique habria que mapearle la pagina que termina justo en 0x08003000, osea 0x08003000 - el tamano de una pagina (4k)
    mmu_map_page(cr3, TASK_STACK_BASE - PAGE_SIZE, mmu_next_free_user_page(), 7);  // attrs = 7 = 0111 (Supervisor, Presente, Lectura y Escritura)
    // "la memoria para la pila de la tarea debe obtenerse del area libre de tareas" entonces la pedimos con mmu_next_free_user_page
    // (no es del area libre del kernel)

    return directoryStart;

}



