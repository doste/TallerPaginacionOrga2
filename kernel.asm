; ** por compatibilidad se omiten tildes **
; ==============================================================================
; TALLER System Programming - ORGANIZACION DE COMPUTADOR II - FCEN
; ==============================================================================

%include "print.mac"
global start


; COMPLETAR - Agreguen declaraciones extern según vayan necesitando
extern GDT_DESC
extern screen_draw_layout
extern idt_init
extern IDT_DESC
extern pic_reset
extern pic_enable
extern mmu_init_kernel_dir
extern mmu_map_page
extern mmu_unmap_page
extern copy_page
extern mmu_init_task_dir

; COMPLETAR - Definan correctamente estas constantes cuando las necesiten
%define CS_RING_0_SEL 0x8   
%define DS_RING_0_SEL 0x18 
%define KERNEL_PAGE_DIR 0x00025000  


BITS 16
;; Saltear seccion de datos
jmp start

;;
;; Seccion de datos.
;; -------------------------------------------------------------------------- ;;
start_rm_msg db     'Iniciando kernel en Modo Real'
start_rm_len equ    $ - start_rm_msg

start_pm_msg db     'Iniciando kernel en Modo Protegido'
start_pm_len equ    $ - start_pm_msg

;;
;; Seccion de código.
;; -------------------------------------------------------------------------- ;;

;; Punto de entrada del kernel.
BITS 16
start:
    ; COMPLETAR - Deshabilitar interrupciones

    cli 
    ; Cambiar modo de video a 80 X 50
    mov ax, 0003h
    int 10h ; set mode 03h
    xor bx, bx
    mov ax, 1112h
    int 10h ; load 8x8 font

    ; COMPLETAR - Imprimir mensaj// COMPLETAR: Interrupciones de reloj y tecladoe de bienvenida - MODO REAL
    ; (revisar las funciones definidas en print.mac y los mensajes se encuentran en la
    ; sección de datos)
    print_text_rm start_rm_msg,start_rm_len, 0xF, 0,0
    

    ; COMPLETAR - Habilitar A20
    ; (revisar las funciones definidas en a20.asm)
    call A20_enable
    ; COMPLETAR - Cargar la GDT
    lgdt [GDT_DESC] 
    ; COMPLETAR - Setear el bit PE del registro CR0
    mov eax,CR0
    or eax,1
    mov cr0,eax
    ; COMPLETAR - Saltar a modo protegido (far jump)
    ; (recuerden que un far jmp se especifica como jmp CS_selector:address)
    ; Pueden usar la constante CS_RING_0_SEL definida en este archivo
    jmp CS_RING_0_SEL:modo_protegido

BITS 32
modo_protegido:
    ; COMPLETAR - A partir de aca, todo el codigo se va a ejectutar en modo protegido
    ; Establecer selectores de segmentos DS, ES, GS, FS y SS en el segmento de datos de nivel 0
    ; Pueden usar la constante DS_RING_0_SEL definida en este archivo
    mov ax,DS_RING_0_SEL
    mov ds, ax
    mov gs,ax
    mov fs,ax
    mov ss,ax
    ; COMPLETAR - Establecer el tope y la base de la pila
    mov ebp,0x25000
    mov esp,0x25000
    ; COMPLETAR - Imprimir mensaje de bienvenida - MODO PROTEGIDO
    print_text_pm start_pm_msg,start_pm_len, 0xF, 2,0
    ;xchg bx,bx
    ; limpiamos la pantalla:
    call screen_draw_layout

    ;inicializamos la IDT y usamos lidt para cargar la IDT en memoria
    xchg bx,bx
    call idt_init
    lidt [IDT_DESC]
    ;xchg bx,bx

    ;remapeamos pic
    call pic_reset
    ;habilitamos pic
    call pic_enable
    

    ;xchg bx,bx
    ;int 88
    ;xchg bx,bx

    ;habilitamos paginacion del kernel
    call mmu_init_kernel_dir   ; inicializamos el directorio de paginas
    mov eax, KERNEL_PAGE_DIR
    mov CR3, eax                ; a cr3 le pasamos la direcc del directorio de paginas (0x00025000, la misma que usamos al definir la funcion mmu_init_kernel_dir)
    mov eax,CR0
    or eax,0x80000000
    mov CR0,eax                 ; prendemos el bit mas signif de cr0 lo cual habilita la paginacion

    xchg bx,bx


    ; TEST mmu_map_page:
    ; ponele que queremos mapear la pagina 0x0050e027
    ;mmu_map_page(uint32_t cr3, vaddr_t virt, paddr_t phy, uint32_t attrs) 
    push 2              ; attrs
    push 0x00400000     ; phy - lo mapeamos a la primer pagina despues del area del kernel
    push 0x0050e000     ; virt - la pagina que queremos mapear, pero alineada a 4K (queremos ir al principio de la pagina)
    mov eax, cr3
    push eax            ; no podemos pushear de una al cr3
    call mmu_map_page
    add esp, 4*4

    xchg bx,bx

    ; TEST mmu_unmap_page:
    ; ponele que queremos desmapear la pagina 0x0050e027
    ; paddr_t mmu_unmap_page(uint32_t cr3, vaddr_t virt)
    
    push 0x0050e000     ; virt - la pagina que queremos mapear, pero alineada a 4K (queremos ir al principio de la pagina)
    mov eax, cr3
    push eax            ; no podemos pushear de una al cr3
    call mmu_unmap_page
    add esp, 4*2
    
    xchg bx,bx

    ; TEST copy_page
    mov [0x1FF000], DWORD 0xCAFECAFE                  ; 0x1FF000 entra en la parte de IdMapping, asique se supone que esta mapeada a la misma direcc fisica
    ; void copy_page(paddr_t dst_addr, paddr_t src_addr)
    push 0x1FF000 ;src    
    push 0x200000 ;dst
    call copy_page                                    ; copio el contenido de src en dst, osea CAFECAFE ahora tambien estara en la direcc 0x200000
    add esp, 4*2
    mov eax, DWORD [0x200000]                         ; si despues de correr esto, hago regs en bochs, en eax me mostrara que esta CAFECAFE


    xchg bx,bx

    ; TEST mmu_init_task_dir

    ;A modo de prueba, en kernel.asm vamos a construir un mapa de memoria para una tarea ficticia (es decir, cargar
    ;el cr3 de una tarea) e intercambiarlo con el del kernel. Para esto tendrán que usar la función antes construida,
    ;mmu_init_task_dir. Supongan que la tarea se encuentra ubicada en la dirección fisica 0x18000. Una vez hecho el
    ;cambio de cr3, impriman algo en la pantalla y vuelvan a la normalidad. Inspeccionar el mapa de memoria con el
    ;comando info tab con breakpoints una vez que se asigna el CR3 de la tarea y cuando se restituye el CR3 del kernel

    ; paddr_t mmu_init_task_dir(paddr_t phy_start)
    push 0x18000
    call mmu_init_task_dir
    mov esi,CR3             ; en esi se guarda KERNEL_PAGE_DIR (0x00025000)
    mov CR3,eax             ; en eax estara el valor de retorno de la funcion, osea el nuevo valor de cr3, el cr3 asociado a la tarea

    xchg bx,bx

    mov CR3,esi             ; restauro el cr3, para que sea el del kernel

    xchg bx,bx

    ;habilitamos interrupciones
    sti
    ;cuando empezamos a correr este kernel (estabamos en modoreal), lo primero que hicimos fue deshabilitar
    ;las interrupciones, asique ahora debemos habilitarlas nuevamente, usando sti


    ; Ciclar infinitamente 
    mov eax, 0xFFFF
    mov ebx, 0xFFFF
    mov ecx, 0xFFFF
    mov edx, 0xFFFF
    jmp $

;; -------------------------------------------------------------------------- ;;

%include "a20.asm"
