.syntax unified
.thumb
.text

.extern current_task
.extern next_task

.global pendsv_handler
.thumb_func

pendsv_handler:
    mrs     r0, psp
    
    @ Only save current task state if current_task is not null
    ldr     r1, =current_task
    ldr     r1, [r1]
    cbz     r1, load_next        @ if current_task == null, skip save
    
    stmdb   r0!, {r4-r11}
    str     r0, [r1, #0]

load_next:
    ldr     r2, =next_task
    ldr     r2, [r2]
    ldr     r0, =current_task
    str     r2, [r0]

    ldr     r0, [r2, #0]
    ldmia   r0!, {r4-r11}
    msr     psp, r0

    bx      lr
