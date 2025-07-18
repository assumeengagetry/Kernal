/*
 * kernel.ld - x86_64架构内核链接脚本
 * 定义内核各段的加载地址和布局。
 */

ENTRY(_start)

SECTIONS
{
    /* 内核加载地址 */
    . = 0x100000;

    /* Multiboot2 头部 */
    .multiboot2 : ALIGN(8)
    {
        *(.multiboot2)
    }

    /* 代码段 */
    .text : ALIGN(4K)
    {
        *(.text)
        *(.text.*)
    }

    /* 只读数据段 */
    .rodata : ALIGN(4K)
    {
        *(.rodata)
        *(.rodata.*)
    }

    /* 已初始化数据段 */
    .data : ALIGN(4K)
    {
        *(.data)
        *(.data.*)
    }

    /* 未初始化数据段 */
    .bss : ALIGN(4K)
    {
        *(COMMON)
        *(.bss)
        *(.bss.*)
    }

    /* 内核结束标记 */
    __kernel_end = .;

    /* 丢弃不需要的段 */
    /DISCARD/ :
    {
        *(.comment)
        *(.note)
        *(.note.*)
        *(.eh_frame)
        *(.eh_frame_hdr)
    }
}
