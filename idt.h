/* ** por compatibilidad se omiten tildes **
================================================================================
 TALLER System Programming - ORGANIZACION DE COMPUTADOR II - FCEN
================================================================================

  Declaracion de las rutinas de atencion de interrupciones
*/

#ifndef __IDT_H__
#define __IDT_H__

#include "types.h"

// COMPLETAR: Definir INTERRUPT_GATE_TYPE con el valor correspondiente
#define INTERRUPT_GATE_TYPE  14
/*
  esto es para el Tipo de Interrupcion, si vemos en el struct idt_entry_t mas abajo vemos que type tiene 5 bits
  entonces en el caso de una puerta de interrupcion (que es este) esos 5 bits son los que tienen: 0 D 1 1 0 
  y esa D indica: 0: 16bits 1: 32bits , como nosotros ya estamos en modoprotegido estamos en 32, asique la ponemos en 1
  entonces el tipo queda: 0 1 1 1 0 == 14
*/

/* Struct de descriptor de IDT */
typedef struct str_idt_descriptor {
  uint16_t idt_length;
  uint32_t idt_addr;
} __attribute__((__packed__)) idt_descriptor_t;

/* Struct de una entrada de la IDT */
typedef struct {
    uint16_t offset_15_0;
    uint16_t segsel;
    uint8_t reserved : 5;
    uint8_t should_be_zero : 3;
    uint8_t type : 5;
    uint8_t dpl : 2;
    uint8_t present : 1;
    uint16_t offset_31_16;
} __attribute__((__packed__, aligned (8))) idt_entry_t;


extern idt_entry_t idt[];
extern idt_descriptor_t IDT_DESC;

void idt_init(void);

#endif //  __IDT_H__
