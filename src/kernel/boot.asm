;; ////////////////////////////////////////////////////////////////////////// */
;; (c) copyright 2024 Lawrence D. Kern ////////////////////////////////////// */
;; ////////////////////////////////////////////////////////////////////////// */

;; NOTE: Fill out the multiboot header, which will mark the kernel as
;; bootable. The structure of the header is referenced from here:
;;
;; https://www.gnu.org/software/grub/manual/multiboot/multiboot.html

   MULTIBOOT_FLAG_ALIGN        equ 1 << 0
   MULTIBOOT_FLAG_MEMORYINFO   equ 1 << 1
   MULTIBOOT_FLAG_VIDEOINFO    equ 0 << 2
   MULTIBOOT_FLAG_OFFSETSVALID equ 0 << 16

   MULTIBOOT_MAGIC    equ 0x1BADB002
   MULTIBOOT_FLAGS    equ (MULTIBOOT_FLAG_ALIGN | MULTIBOOT_FLAG_MEMORYINFO | MULTIBOOT_FLAG_VIDEOINFO | MULTIBOOT_FLAG_OFFSETSVALID)
   MULTIBOOT_CHECKSUM equ -(MULTIBOOT_MAGIC + MULTIBOOT_FLAGS)

section .multiboot
align 4
   dd MULTIBOOT_MAGIC
   dd MULTIBOOT_FLAGS
   dd MULTIBOOT_CHECKSUM

;; NOTE: The stack on x86 must be 16-byte aligned according to the System V ABI
;; standard and de-facto extensions. The compiler will assume the stack is
;; properly aligned and failure to align the stack will result in undefined
;; behavior.

section .bss
align 16
stack_bottom:
   resb 0x4000 ; 16 KB
stack_top:

;; NOTE: The entry point _start is specified in kernel_linker.ld. This is where
;; the bootloader will begin executing our kernel.

section .text
global _start:function (_start.end - _start)
_start:

;; NOTE: Specify the value of the stack pointer based on the stack_top label we
;; specified above.

   mov esp, stack_top

;; TODO: Load the global descriptor table here.

;; NOTE: Enter the entry point (kernel_main) that we defined in C.

extern kernel_main
   call kernel_main

   cli
.spin:
   hlt
   jmp .spin
.end:
