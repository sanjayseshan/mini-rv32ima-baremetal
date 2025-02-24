OUTPUT_ARCH( "riscv" )
ENTRY( _start)

SECTIONS
{
  . = 0x0;
  .text.init : { *(.text.init) }
  . = ALIGN(0x1000);
  .text : { *(.text) }
  . = ALIGN(0x2000);
  .data : { *(.data) }
  . = ALIGN(0x4000);
  .data.ram_image : {*.data.ram_image}
  .bss : { *(.bss) }
 _end = .;
}
