strlen: ; strlen(unsigned char *)
    ; r1 --> string
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

