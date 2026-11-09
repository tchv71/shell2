;void memswap(void* arg1, void* arg2, uint arg3) {
_memswap:
    push bc
    ; bc = count
    ; hl = to
    ; de = from
memswap_l1:
    ; if(cnt==0) return
    ld a, b
    or c
    jp z, memswap_l2
    ; *dest = *src
    ld a,(de)
    ld (memswap_v1),a
    ld a, (hl)
    ld (de), a
    .db 36h ; mvi m, 0
memswap_v1:
    .db 0    
    ; dest++, src++, cnt--
    inc hl
    inc de
    dec bc
    ; loop
    jp memswap_l1
memswap_l2:
    pop bc
    ret

;void memcpy_back(void* arg1, void* arg2, uint arg3) {
_memcpy_back:
    ; if(cnt==0) return
    ld a, b
    or c
    ret z
    ; start
    push bc
    ; bc = count
    ; de = from
    push hl
    ex  de, hl
    add hl,bc
    ld e, l
    ld d, h
    pop hl
    ; hl = to
    add hl,bc
    ; enter loop
    inc b
    xor a
    or  c
    jp  z, memcpy_back_l2
memcpy_back_l1:
    ; dest--, src--
    dec hl
    dec de
    ; *dest = *src
    ld  a, (de)
    ld  (hl), a
    ; while(cnt)
    dec c
    jp  nz, memcpy_back_l1
memcpy_back_l2:
    dec b
    jp  nz, memcpy_back_l1
    pop bc
    ret

;void print2(uchar* dest, char* text) {
_print2:
    call setVdpAddr
print2_loop:
    ld  a, (de)
    or  a
    ret z
    out (98h), a
    inc de
    jp  print2_loop

;void print2m(uchar* dest, uchar len, char* text) {
_print2m:
    ld   b, a
    call setVdpAddr
print2m_loop:
    ld   a, (de)
    or   a
    jp   z, print2m_ret
    inc  de
print2m_ret:
    out  (98h), a
    dec  b
    jp   nz, print2m_loop
    ret 

;void print2n(uchar* dest, uchar len, char* text) {
_print2n:
    ld   b, a
    call setVdpAddr
print2n_loop:
    ld   a, (de)
    or   a
    jp   z, print2n_ret
    out  (98h), a
    inc  de
    dec  b
    jp   nz, print2n_loop 
print2n_ret:
    ret

;void print2cn(uchar* dest, uchar len, char c) {
_print2cn:
    ld   d, a
    call setVdpAddr
    ld   a, e
print2cn_loop:
    out  (98h), a
    dec  d
    jp   nz, print2cn_loop
    ret

;void fs_init() {
_fs_init:
    LD   (_fs_cmdLine), hl
    ex   de,hl
    ld   (_fs_selfName), hl
    ld   h, b
    ld   l, c
    ld   (fs_entry_n+1), hl
    ret

;uchar fs_exec(const char* fileName, const char* cmdLine) {
_fs_exec:
    xor  a
    jp   _fs_entry

;void fs_entry() {
_fs_entry:
fs_entry_n:
    JP 0000h

;uchar fs_findfirst(const char* path, FileInfo* dest, uint size) {
;  if(path[0] == '/') path++;
_fs_findfirst:
    ld   a,(hl)
    cp   '/'
    jp   nz, fs_ff0
    inc  hl
fs_ff0:
    ; hl = fs_findfirst_3
    push de
    push bc
    pop  de
    pop  bc
    ld   a, 1
    CALL _fs_entry
    ld   (_fs_low), hl
    ret

;uchar fs_open0(const char* name, uchar mode) {
_fs_open0:
    ; a = fs_open0_2
    ld   D, A 
    ld   A, 2
    jp   _fs_entry


;uchar fs_write(const void* buf, uint size) {
_fs_write:
    ; hl = fs_write_2
    ex   de, hl
    LD   A, 5
    jp   _fs_entry ; HL-размер, DE-адрес / A-код ошибки

;uchar fs_seek(uint low, uint high, uchar mode) {
_fs_seek:
    ; a = fs_seek_3
    ld   B, A
    ld   A, 3
    CALL _fs_entry ; B-режим, DE:HL-имя файла / A-код ошибки, DE:HL-позиция
    ld   (_fs_low), hl
    ex   de,hl
    ld   (_fs_high), hl
    ret

;uchar fs_read(void* buf, uint size) {
_fs_read:
    ; hl = fs_read_2
    ex   de, hl
    ld   A, 4
    CALL _fs_entry ; HL-размер, DE-адрес / HL-сколько загрузили, A-код ошибки
    ld   (_fs_low), hl
    ret

;uchar fs_move(const char* from, const char* to) {
_fs_move:
    ld  A, 6
    jp  _fs_entry ; HL-из, DE-в / A-код ошибки

;void clrscr() {
_clrscr:
    ld  c, 1Fh
    jp  0F809h

;void putch(char) {
_putch:
    ld   c, a
    jp   0F809h

_main:
	CALL    _fs_init
	JP	    _main2

;void fillRect(uchar* a, uchar w, uchar h, uchar c) {
_fillRect:
    push bc
    ;lda fillRect_2
    ld   e, a
    ld   a, 80;radio86rkVideoBpl
    ld   b, c
    ld   c, a
    ;ld   b, 0
    ;lhld fillRect_1
    ;lda fillRect_3
    ld   d, e
fillRect_l1:  
    ;lda fillRect_2
    push hl
    call setVdpAddr
    ;lda  fillRect_4
    ld   a,b

fillRect_l0:
    out  (98h), a
    dec  e
    jp   nz, fillRect_l0
    pop  hl
    push bc
    ld   b,0
    add  hl, bc
    pop  bc
    dec  d
    jp   nz, fillRect_l1
    pop  bc
    ret

;void hLine(uchar* a, uchar c, uchar w);
_hLine:
    ld   d, a
    ; mov e, a
    ; lda hLine_2
    ; lhld hLine_1
    call setVdpAddr
    ld   a, d
hLine_l0 :
    OUT  (98h), a
    ; mov m, d
    dec  e
    jp	 nz, hLine_l0
    ret

;void vLine(uchar* a, uchar h, uchar c) {
_vLine:
    ld   d,e
    ld   e, a
    ld   bc,80
vLine_l0:
    call setVdpAddr
    ld   a, d
    OUT  (98h), a
    add   hl, bc
    dec   e
    jp	  nz, vLine_l0
    ret

setVdpAddr:
    di
    ld   a,l
    out  (98h + 1), a
    ld   a,h
    or   40h
    out  (98h + 1), a
    ei
    ret
