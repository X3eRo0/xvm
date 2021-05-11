strlen: ; strlen(unsigned char *)
    ; r1 --> string
    xor  $r2, $r2
    movb $r2, [$r1]
    test $r2, $r2
    jz   strlen_end
    mov  $r0, #1

    strlen_loop:
        mov  $r8, $r0
        inc  $r0
        mov  $r4, $r1   ; $r4 = &str[0]
        add  $r4, $r0   ; $r4 = &str[$r0]
        dec  $r4        ; $r4 = &str[$r0-1]
        movb $r2, [$r4] ; $r2 = *$r4
        test $r2, $r2
        jnz  strlen_loop
        mov  $r0, $r8
        ret

    strlen_end:
        xor $r8, $r8
        mov $r0, $r8
        ret

strncmp: ; strncmp(unsigned char *str1, unsigned char *str2, u32 len)
    test    $r3, $r3
    jz      .strncmp_L4
    xor     $r6, $r6
    xor     $r9, $r9
    jmp     .strncmp_L3
    .strncmp_L4:
        mov     $r7, $r2
        add     $r7, $r6 
        movb    $ra, [$r7]
        xor     $r0, $ra
        inc     $r6
        or      $r9, $r0
        cmp     $r3, $r6
        jz      .strncmp_L1
    .strncmp_L3:
        mov     $r7, $r1
        add     $r7, $r6
        movb    $r0, [$r7]
        test    $r0, $r0
        jnz     .strncmp_L4
    .strncmp_L1:
        mov     $r0, $r9
        ret
    .strncmp_L4:
        xor     $r9, $r9
        jmp     .strncmp_L1


int2str:
    ; convert int to str
    ; rdi --> number
    ; rsi --> buffer
    push    $bp
    mov     $bp, $sp
    mov     $r8, $r1
    mov     $rc, $r1
    mov     $r9, $r2
    .int2str_L1:
        
        mov     $rb, $r8
        div     $rb, #0x0a
        mov     $rb, $r1
        push    $rb
        div     $r8, #0x0a
        test    $r8, $r8
        jnz     .int2str_L1
    
    mov     $r8, $rc
    xor     $ra, $ra

    .int2str_L2:
        pop     $ra
        add     $ra, #0x30
        movb    [$r9], $ra
        inc     $r9
        div     $r8, #0x0a
        test    $r8, $r8
        jnz     .int2str_L2

    inc     $r9
    movb    [$r9], #0x00

    .int2str_L3:
        mov     $r0, $r2
        mov     $sp, $bp
        pop     $bp
        ret


str2int:
    push  $bp
    mov   $bp, $sp
    call  strlen
    mov   $r8, $r0
    mov   $r9, $r8
    xor   $rc, $rc
    xor   $r4, $r4
    mov   $r9, $r8

    .str2int_L1:

        dec   $r9
        cmp   $rc, $r8             ; compare if $rc == length of string
        jz    .str2int_L1_end        ; if equal end the loop
        movb  $r6, [$r1]
        inc   $r1
        inc   $rc
        cmp   $r6, #0x30          ; check if the value is less than #0x30
        jb    .str2int_L3     ; jump to .str2int_L3
        cmp   $r6, #0x39          ; check if the value is larger than #0x39
        ja    .str2int_L3     ; jump to .str2int_L3
        sub   $r6, #0x30          ; if everything is good then subtract #0x30
        xor   $rd, $rd
        mov   $r5, #0x1

        .str2int_L2:
          cmp   $rd, $r9
          jz    .str2int_L2_end
          mul   $r5, #0xa
          inc   $rd
          jmp   .str2int_L2

        .str2int_L2_end:

        mul     $r5, $r6
        add     $r4, $r5
        jmp .str2int_L1


.str2int_L1_end:
    mov $sp, $bp
    pop $bp
    mov $r0, $r4
    ret

.str2int_L3:
    mov $r1, str2int_ERR
    call puts
    hlt

.section .data

str2int_ERR:
    .asciz "abort [xvm-lib][str2int] Input is not a decimal number"