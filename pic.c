/* ** por compatibilidad se omiten tildes **
================================================================================
 TALLER System Programming - ORGANIZACION DE COMPUTADOR II - FCEN
================================================================================

  Rutinas del controlador de interrupciones.
*/
#include "pic.h"

#define PIC1_PORT 0x20
#define PIC2_PORT 0xA0

static __inline __attribute__((always_inline)) void outb(uint32_t port,
                                                         uint8_t data) {
  __asm __volatile("outb %0,%w1" : : "a"(data), "d"(port));
}
/*
void pic_finish1(void) { outb(0x20, 0x20); }
void pic_finish2(void) {
  outb(0x20, 0x20);
  outb(0xA0, 0x20);
}
*/
void pic_finish1(void) { 
  outb(PIC1_PORT, 0x20); 
}
//0x20 es la OCW2 (para fin de interrupcion) (ver teorica)
void pic_finish2(void) {
  outb(PIC1_PORT, 0x20);
  outb(PIC2_PORT, 0x20);
}

// COMPLETAR: implementar pic_reset()
void pic_reset() {
  // en nuestro caso, tanto para pic1 como pic2, la ICW1 sera 0x11 (ver apunte teorica para ver por que)
  outb(PIC1_PORT, 0x11);
  outb(PIC2_PORT, 0x11);

  // la siguiente palabra de inicializacion (ICW2) se escribira en 0x21 para pic1, y en 0xA1 para pic2
  // en ICW2, le pasamos el tipo de interrupcion, osea desde queremos que arranque cada pic
  // el pic1 lo queremos remapear a partir de la 32 (osea 0x20)
  outb(PIC1_PORT + 1, 0x20);
  // y el pic2 a partir de la 40 (osea 0x28)
  outb(PIC2_PORT + 1, 0x28);

  // la ICW3 sirve para configurar la relacion de master y slave, osea hace la vinculacion logica entre master y slave
  // en nuestro caso, queremos que el pic1 sea master y que en su IRQ2 tenga conectado al pic2
  // entonces a la ICW3 del pic1 deberia ser: 0 0 0 0 0 1 0 0 == 0x4
  outb(PIC1_PORT + 1, 0x4);
  // ahora para pic2, sabemos que debe ser slave, y lo que hay que decirle es a que linea del master esta conectado
  // en nuestro caso, sabemos que ira conectado al IR2 del master, entonces el ICW3 del pic2 sera 0 0 0 0 0 0 1 0 == 0x2
  outb(PIC2_PORT + 1, 0x2);

  // la ICW4 sera 0 0 0 0 0 0 0 1 para ambos pic (ver teorica porque), entonces:
  outb(PIC1_PORT + 1, 0x1);
  outb(PIC2_PORT + 1, 0x1);

  // la OCW1 es para setear la mascara, ponemos 1 en cada bit que queramos setear
  // ?????????????????????????????
  outb(PIC1_PORT + 1, 0xFF);
  // no hay OCW1 para pic2



}

// habilita los pic para que generen interrupciones
void pic_enable() {
  outb(PIC1_PORT + 1, 0x00);
  outb(PIC2_PORT + 1, 0x00);
}

// deshabilita los pic para que generen interrupciones
void pic_disable() {
  outb(PIC1_PORT + 1, 0xFF);
  outb(PIC2_PORT + 1, 0xFF);
}
