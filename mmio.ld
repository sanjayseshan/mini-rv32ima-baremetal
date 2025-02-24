OUTPUT_ARCH( "riscv" )
ENTRY( _start)

__heap_size    = 0x400000;  /* required amount of heap */
__stack_size  = 0x400000;  /* required amount of stack */

SECTIONS
{
  . = 0x0;
  .text.init : { *(.text.init) }
  . = ALIGN(0x1000);
  .text : { *(.text) }
  . = ALIGN(0x2000);
  .data : { *(.data) }
  . = ALIGN(0x800000);
  .data.ram_image : {*.data.ram_image}
  .bss : { *(.bss) }

  .heap :
    {
      . = ALIGN(4);
      PROVIDE ( end = . );
      _sheap = .;
      . = . + __heap_size;
      . = ALIGN(4);
      _eheap = .;
    }

    . = ALIGN(0xffffff00);
    .stack :
    {
      . = ALIGN(4);
      _estack = .;
      . = . + __stack_size;
      . = ALIGN(4);
      _sstack = .;
    }


 _end = .;
}
