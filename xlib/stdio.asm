; standard io functions

print:
    
    push    $bp
    mov     $bp, $sp
    call    strlen
    mov     $r2, $r1
    mov     $r5, $r0
    mov     $r1, STDOUT
    call    write
    mov     $sp, $bp
    pop     $bp
    ret

puts:

    push    $bp
    mov     $bp, $sp
    sub     $sp, #0x04
    mov     [$bp-#0x04], #0x0a
    call    strlen
    mov     $r5, $r0
    mov     $r2, $r1
    mov     $r1, STDOUT
    call    write
    mov     $r5, #0x01
    mov     $r2, $bp
    sub     $r2, #0x04
    mov     $r1, STDOUT
    call    write
    mov     $sp, $bp
    pop     $bp
    ret
    

read:
    
    push    $bp
    mov     $bp, $sp
    mov     $r0, SYS_READ
    syscall
    mov     $sp, $bp
    pop     $bp
    ret

write:

    push    $bp
    mov     $bp, $sp
    mov     $r0, SYS_WRITE
    syscall
    mov     $sp, $bp
    pop     $bp
    ret

gets:
    push    $bp
    mov     $bp, $sp
    mov     $r5, $r2
    mov     $r2, $r1
    mov     $r1, STDIN
    mov     $r0, SYS_READ
    syscall
    mov     $sp, $bp
    pop     $bp
    ret