	.setcpu 6
	.code
	.export _kr_serial_open
	.export _kr_serial_line
	.export _kr_serial_put
	.export _kr_serial_send
	.export _kr_serial_close

; SYS000 must be assigned to CRT1 before the program starts.
_kr_serial_open:
	stx (-s)
	.byte 0x66,0x39,3
	.word __crt1
	.byte 0
	ldbb (__crt1)
	clrb bh
	ldx (s+)
	rsr

_kr_serial_line:
	stx (-s)
	.byte 0x66,0x34
	.word __crt1, __osfmt75, __line, 0
	ldb __line
	ldx (s+)
	rsr

_kr_serial_put:
	stx (-s)
	ldb 4(s)
	ldab 0x80
	orib al,bl
	stbb (__char)
	.byte 0x66,0x31
	.word __crt1, __osfmt1, __char, 0
	ldx (s+)
	rsr

; Send the complete reply using a single WRITEN with a dynamic C format.
_kr_serial_send:
	stx (-s)
	ldb 4(s)
	stb (__reply_length)
	.byte 0x66,0x31
	.word __crt1, __reply_format, _kr_reply_buffer, 0
	ldx (s+)
	rsr

_kr_serial_close:
	stx (-s)
	.byte 0x66,0x3a,0x0a
	.word __crt1
	ldbb (__crt1)
	clrb bh
	ldx (s+)
	rsr

	.data
__crt1:
	.byte 0,0
	.ds 11
	.byte 2
	.ds 16
__char:
	.byte 0,0
__reply_format:
	.byte 2
__reply_length:
	.word 1
	.byte 0

	.bss
__line:
	.ds 76
