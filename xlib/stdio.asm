; standard io functions

print:
    
    push    $bp
    mov     $bp, $sp
    call    strlen
    mov     $r2, $r1
    mov     $r3, $r0
    mov     $r1, STDOUT
    call    write
    mov     $sp, $bp
    pop     $bp
    ret

puts:

    push    $bp
    mov     $bp, $sp
    add     $sp, #0x04
    mov     [$bp], #0x0a00
    call    strlen
    mov     $r3, $r0
    mov     $r2, $r1
    mov     $r1, STDOUT
    call    write
    mov     $r3, #0x02
    mov     $r2, $bp
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
    mov     $r3, $r2
    mov     $r2, $r1
    mov     $r1, STDIN
    mov     $r0, SYS_READ
    syscall
    mov     $sp, $bp
    pop     $bp
    ret

system:
    push    $bp
    mov     $bp, $sp
    mov     $r0, SYS_EXEC
    syscall
    mov     $sp, $bp
    pop     $bp
    ret