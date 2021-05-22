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


reverse:                                ; @reverse
; %bb.0:
    push    $bp
    mov    $bp, $sp
    mov    [$bp - #0x10], $r1
    mov    [$bp - #0x18], $r2
    mov    [$bp - #0x1c], $r5
    mov    [$bp - #0x20], #0x0
reverse_bb0_1:                                ; =>This Inner Loop Header: Depth=1
    mov    $r0, [$bp - #0x20]
    cmp    $r0, [$bp - #0x1c]
    jae    reverse_bb0_4
; %bb.2:                                ;   in Loop: Header=BB0_1 Depth=1
    mov    $r0, [$bp - #0x10]
    mov    $r4, [$bp - #0x20]
    mov    $r5, $r4
    add     $r0, $r5
    movb    $r2, [$r0]
    mov    $r0, [$bp - #0x18]
    mov    $r4, [$bp - #0x1c]
    sub    $r4, [$bp - #0x20]
    sub    $r4, #0x1
    mov    $r4, $r4
    mov    $r5, $r4
    add     $r0, $r5
    movb    [$r0], $r2
; %bb.3:                                ;   in Loop: Header=BB0_1 Depth=1
    mov    $r0, [$bp - #0x20]
    add    $r0, #0x1
    mov    [$bp - #0x20], $r0
    jmp    reverse_bb0_1
reverse_bb0_4:
    mov    $r0, [$bp - #0x4]
    pop    $bp
    ret
                                        ; -- End function
str2int:                                ; @str2int
; %bb.0:
    push    $bp
    mov    $bp, $sp
    sub    $sp, #0x20
    mov    [$bp - #0x8], $r1
    mov    $r1, [$bp - #0x8]
    call    strlen
                                        ; kill: def $$r0 killed $$r0 killed $$r0
    mov    [$bp - #0xc], $r0
    mov    [$bp - #0x14], #0x0
    mov    [$bp - #0x18], #0x1
    mov    [$bp - #0x10], #0x0
str2int_bb1_1:                                ; =>This Inner Loop Header: Depth=1
    mov    $r0, [$bp - #0x10]
    cmp    $r0, [$bp - #0xc]
    jae    str2int_bb1_4
; %bb.2:                                ;   in Loop: Header=BB1_1 Depth=1
    mov    $r0, [$bp - #0x14]
    mov    $r4, [$bp - #0x8]
    mov    $r5, [$bp - #0xc]
    mov    $r2, [$bp - #0x10]
    add    $r2, #0x1
    sub    $r5, $r2
    mov    $r5, $r5
    mov    $r1, $r5
    add     $r4, $r1
    movb    $r5, [$r4]
    sub    $r5, #0x30
    mul    $r5, [$bp - #0x18]
    add    $r0, $r5
    mov    [$bp - #0x14], $r0
    mul   [$bp - 24], #0xa
    mov $r0,   [$bp - 24]
    mov    [$bp - #0x18], $r0
; %bb.3:                                ;   in Loop: Header=BB1_1 Depth=1
    mov    $r0, [$bp - #0x10]
    add    $r0, #0x1
    mov    [$bp - #0x10], $r0
    jmp    str2int_bb1_1
str2int_bb1_4:
    mov    $r0, [$bp - #0x14]
    add    $sp, #0x20
    pop    $bp
    ret
                                        ; -- End function
int2str:                                ; @int2str
; %bb.0:
    push    $bp
    mov    $bp, $sp
    sub    $sp, #0x30
    mov    [$bp - #0x8], $r1
    mov    [$bp - #0xc], $r2
    mov    [$bp - #0x18], #0x0
    mov    [$bp - #0x1c], #0x0
    test   $r2, $r2
    jz     int2str_ret_0
    mov    $r0, [$bp - #0xc]
    mov    [$bp - #0x1c], $r0
int2str_bb2_1:                                ; =>This Inner Loop Header: Depth=1
    cmp    [$bp - #0x1c], #0x0
    jz    int2str_bb2_3
; %bb.2:                                ;   in Loop: Header=BB2_1 Depth=1
    mov    $r0, [$bp - #0x18]
    add    $r0, #0x1
    mov    [$bp - #0x18], $r0
    mov    $r0, [$bp - #0x1c]
    xor    $r5, $r5
    mov    $r4, #0xa
    div    $r0, $r4
    mov    [$bp - #0x1c], $r0
    jmp    int2str_bb2_1
int2str_bb2_3:
    mov    [$bp - #0x10], #0x0
int2str_bb2_4:                                ; =>This Inner Loop Header: Depth=1
    mov    $r0, [$bp - #0x10]
    cmp    $r0, [$bp - #0x18]
    jae    int2str_bb2_7
; %bb.5:                                ;   in Loop: Header=BB2_4 Depth=1
    mov    $r0, [$bp - #0xc]
    xor    $r5, $r5
    mov    $r4, #0xa
    div    $r0, $r4
    mov    [$bp - #0x14], $r5
    mov    $r5, [$bp - #0xc]
    mov    $r0, $r5
    xor    $r5, $r5
    div    $r0, $r4
    mov    [$bp - #0xc], $r0
    mov    $r0, [$bp - #0x14]
    add    $r0, #0x30
                                        ; kill: def $al killed $al killed $$r0
    mov    $r2, [$bp - #0x8]
    mov    $r4, [$bp - #0x18]
    mov    $r1, [$bp - #0x10]
    add    $r1, #0x1
    sub    $r4, $r1
    mov    $r4, $r4
    mov    $r8, $r4
    add     $r2, $r8
    movb    [$r2], $r0
; %bb.6:                                ;   in Loop: Header=BB2_4 Depth=1
    mov    $r0, [$bp - #0x10]
    add    $r0, #0x1
    mov    [$bp - #0x10], $r0
    jmp    int2str_bb2_4
int2str_bb2_7:
    mov    $r0, [$bp - #0x8]
    mov    $r4, [$bp - #0x18]
    mov    $r5, $r4
    add     $r0, $r5
    movb    [$r0], #0x0
    mov    $sp, $bp
    pop    $bp
    ret

int2str_ret_0:
    movb [$r1], #0x30
    movb [$r1+#1], #0x00
    mov    $sp, $bp
    pop    $bp
    ret

memset:                                ; @memset
; %bb.0:
    mov $r6, $r1
    xor $r7, $r7
memset_loop:
    cmp $r7, $r5
    jae .MMST_1
    movb [$r6], $r2
    inc $r7
    inc $r6
    jmp memset_loop
.MMST_1:
    mov $r0, $r1
    ret

memcpy:                                ; @memcpy
; %bb.0:
    mov $r6, $r1
    xor $r7, $r7
memcpy_loop:
    cmp $r7, $r5
    jae .MMST_1
    movb [$r6], [$r2]
    inc $r2
    inc $r7
    inc $r6
    jmp memcpy_loop
.MMST_1:
    mov $r0, $r1
    ret