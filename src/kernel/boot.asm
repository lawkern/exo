;;; (c) copyright 2024 Lawrence D. Kern ///////////////////////////////////// */

    ;; NOTE: Fill out the multiboot header, which will mark the kernel as
    ;; bootable. The structure of the header is referenced from here:
    ;; https://www.gnu.org/software/grub/manual/multiboot/multiboot.html

    global start:function (start.end - start)
    extern kernel_main

    MULTIBOOT_FLAG_ALIGN        equ 1 << 0
    MULTIBOOT_FLAG_MEMORYINFO   equ 1 << 1
    MULTIBOOT_FLAG_VIDEOINFO    equ 0 << 2
    MULTIBOOT_FLAG_OFFSETSVALID equ 0 << 16

    MULTIBOOT_MAGIC    equ 0x1BADB002
    MULTIBOOT_FLAGS    equ (MULTIBOOT_FLAG_ALIGN | MULTIBOOT_FLAG_MEMORYINFO | MULTIBOOT_FLAG_VIDEOINFO | MULTIBOOT_FLAG_OFFSETSVALID)
    MULTIBOOT_CHECKSUM equ -(MULTIBOOT_MAGIC + MULTIBOOT_FLAGS)

    CODE_SEGMENT equ 0x08
    DATA_SEGMENT equ 0x10

;;; ///////////////////////////////////////////////////////////////////////// */
    section .multiboot
    align 4

    dd MULTIBOOT_MAGIC
    dd MULTIBOOT_FLAGS
    dd MULTIBOOT_CHECKSUM

;;; ///////////////////////////////////////////////////////////////////////// */
    section .text

start:
    ;; NOTE: Specify the value of the stack pointer based on the stack_top label we
    ;; specified above.
    mov esp, stack_top

    ;; NOTE: Clear interrupt flag.
    cli

    ;; NOTE: Load global descriptor table.
    lgdt [gdt_descriptor]

    ;; NOTE: Enable protected mode via control register.
    mov eax, cr0
    or eax, 1
    mov cr0, eax

    ;; NOTE: Update segment registers.
    mov ax, DATA_SEGMENT
    mov ds, ax
    mov es, ax
    mov ss, ax

    ;; NOTE: Flush instruction pipeline.
    jmp CODE_SEGMENT:start.protected_mode_begin

    ;; NOTE: The processor should be in protected mode after this point.
    .protected_mode_begin:

    call kernel_main

    .spin:
    hlt
    jmp .spin

    .end:

;;; ///////////////////////////////////////////////////////////////////////// */
    section .rodata
    align 8

    ;; NOTE: The specific meanings for each field are described at:
    ;; https://wiki.osdev.org/Global_Descriptor_Table
    ;; https://wiki.osdev.org/GDT_Tutorial

gdt:
    ;; NOTE: Null descriptor
    dd 0x0
    dd 0x0

    ;; NOTE: Code segment descriptor
    dw 0xFFFF                   ; Limit0
    dw 0x0                      ; Base0
    db 0x0                      ; Base1
    db 10011010b                ; Access byte
    db 11001111b                ; Flags and Limit1
    db 0x0                      ; Base2

    ;; NOTE: Data segment descriptor
    dw 0xFFFF                   ; Limt0
    dw 0x0                      ; Base0
    db 0x0                      ; Base1
    db 10010010b                ; Access byte
    db 11001111b                ; Flags and Limit1
    db 0x0                      ; Base2

gdt_descriptor:
    dw gdt_descriptor - gdt - 1 ; Size (0 to 15)
    dd gdt                      ; Offset (16 to 48 in 32-bit mode)

;;; ///////////////////////////////////////////////////////////////////////// */
    section .data

;;; ///////////////////////////////////////////////////////////////////////// */
    section .bss
    align 16

    ;; NOTE: The stack on x86 must be 16-byte aligned according to the System V ABI
    ;; standard and de-facto extensions. The compiler will assume the stack is
    ;; properly aligned and failure to align the stack will result in undefined
    ;; behavior.

stack_bottom:
    resb 0x4000 ; NOTE: 16 KB of stack space.
stack_top:
