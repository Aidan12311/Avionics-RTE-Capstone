MEMORY
{
    FLASH (rx)  : ORIGIN = 0x08000000, LENGTH = 2048K
    RAM   (rwx) : ORIGIN = 0x20000000, LENGTH = 128K
    CCM   (rwx) : ORIGIN = 0x10000000, LENGTH = 64K
}

_estack = ORIGIN(RAM) + LENGTH(RAM);

SECTIONS
{
    .text :
    {
        KEEP(*(.isr_vector))
        *(.text*)
        *(.rodata*)

        . = ALIGN(4);
        _init_array_start = .;
        KEEP(*(.init_array*))
        _init_array_end = .;

        . = ALIGN(4);
        _sidata = .;
    } > FLASH

    .data :
    {
        . = ALIGN(4);
        _sdata = .;
        *(.data*)
        . = ALIGN(4);
        _edata = .;
    } > RAM AT> FLASH

    .bss :
    {
        . = ALIGN(4);
        _sbss = .;
        *(.bss*)
        *(COMMON)
        . = ALIGN(4);
        _ebss = .;
    } > RAM

    .ccm (NOLOAD) :
    {
        . = ALIGN(4);
        *(.ccm*)
        . = ALIGN(4);
    } > CCM

    /DISCARD/ :
    {
        *(.ARM.exidx*)
        *(.ARM.extab*)
        *(.fini_array*)
    }
}

ASSERT(SIZEOF(.text) + SIZEOF(.data) <= LENGTH(FLASH),
       "ERROR: Flash overflow — code + data exceeds 2MB")
ASSERT(SIZEOF(.data) + SIZEOF(.bss) <= LENGTH(RAM),
       "ERROR: RAM overflow — data + bss exceeds 128KB")
