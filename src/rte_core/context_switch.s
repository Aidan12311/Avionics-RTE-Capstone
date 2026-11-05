    .syntax unified
    .thumb
    .text

    .extern current_task
    .extern next_task

    .global pendsv_handler
    .thumb_func

pendsv_handler:
    mrs     r0, psp                     @ r0 = outgoing task's PSP

    ldr     r1, =current_task
    ldr     r1, [r1]                    @ r1 = outgoing TCB (null on first launch)
    cbz     r1, load_next               @ nothing to save on first launch

    tst     lr, #0x10                   @ EXC_RETURN bit 4 clear = FPU frame in use
    it      eq
    vstmdbeq r0!, {s16-s31}             @ save callee-saved FPU regs only if used

    stmdb   r0!, {r4-r11, lr}           @ save r4-r11 and EXC_RETURN
    str     r0, [r1, #0]                @ outgoing->_sp = r0

load_next:
    ldr     r2, =next_task
    ldr     r2, [r2]                    @ r2 = incoming TCB
    ldr     r3, =current_task
    str     r2, [r3]                    @ current_task = incoming

    ldr     r0, [r2, #0]                @ r0 = incoming->_sp
    ldmia   r0!, {r4-r11, lr}           @ restore r4-r11 and EXC_RETURN

    tst     lr, #0x10
    it      eq
    vldmiaeq r0!, {s16-s31}             @ restore FPU regs if the frame was extended

    msr     psp, r0
    bx      lr