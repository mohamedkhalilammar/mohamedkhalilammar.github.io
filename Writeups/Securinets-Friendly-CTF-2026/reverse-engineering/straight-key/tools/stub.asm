; Decryptor stub for pack_pe.py. Assembled with `nasm -f bin`.
;
; Runs as the packed image's entry point, before any Go runtime code exists. Walks a table
; of (rva, len, seed) triples appended by the packer, decrypts each region in place, then
; jumps to the original entry point.
;
; Position independent: it recovers the image base from its own RIP rather than trusting a
; fixed ImageBase, so it survives a relocated load even though the packer turns ASLR off.
; No imports, no stack use, no API calls -- there is nothing here for an import-table or
; string scan to find.
;
; Three placeholders are patched by the packer, located by magic value:
;   0xAAAA1111  this stub's RVA
;   0xBBBB2222  the original entry point RVA
;   0xCCCC3333  start of the descriptor table (the packer appends it here)

BITS 64
default rel

stub_start:
        lea     r11, [rel stub_start]
        mov     eax, 0xAAAA1111
        sub     r11, rax                    ; r11 = image base

        mov     r13d, 0x811C9DC5            ; running carry, folded from plaintext
        lea     rsi, [rel table]

.section_loop:
        mov     eax, [rsi]                  ; rva
        test    eax, eax
        jz      .all_done
        mov     r8d, [rsi + 4]              ; length
        mov     r9d, [rsi + 8]              ; stored seed
        lea     rdi, [r11 + rax]            ; target = base + rva

        ; state = seed ^ rotl32(rva, 7) ^ (len * 0x9E3779B1) ^ carry
        ; Folding the region's own geometry into the key means a static unpacker cannot
        ; just lift the stored 32 bits; it has to reproduce the schedule as well. The carry
        ; makes every region depend on the plaintext of every region before it, so there is
        ; no way to decrypt .rdata for its strings without doing .text correctly first.
        mov     edx, eax
        rol     edx, 7
        xor     r9d, edx
        mov     edx, r8d
        imul    edx, edx, 0x9E3779B1
        xor     r9d, edx
        xor     r9d, r13d

        xor     r10d, r10d                  ; i = 0

.byte_loop:
        test    r8d, r8d
        jz      .section_done

        ; xorshift32
        mov     edx, r9d
        mov     ecx, edx
        shl     ecx, 13
        xor     edx, ecx
        mov     ecx, edx
        shr     ecx, 17
        xor     edx, ecx
        mov     ecx, edx
        shl     ecx, 5
        xor     edx, ecx
        mov     r9d, edx
        shr     edx, 16                     ; dl = keystream byte

        ; plain = ror8(cipher, i & 7) ^ k
        mov     bl, [rdi]
        mov     ecx, r10d
        and     cl, 7
        ror     bl, cl
        xor     bl, dl
        mov     [rdi], bl

        movzx   eax, bl
        xor     r13d, eax
        rol     r13d, 5

        inc     rdi
        inc     r10d
        dec     r8d
        jmp     .byte_loop

.section_done:
        add     rsi, 12
        jmp     .section_loop

.all_done:
        mov     eax, 0xBBBB2222
        add     r11, rax
        jmp     r11

        align   4
table:
        dd      0xCCCC3333
