; ReC98
; -----
; ZUNINIT.COM, the first program packed into TH02's ZUN.COM (and reused as
; TH03's `-2` option). ZUN's hand-written resident interrupt handlers:
;
; - INT 59h: shows one of TH02's fatal error messages on the text layer and
;   waits for the Return key. AX = message ID.
; - INT 06h / INT 05h: the PC-98's STOP and COPY keys. Instead of their usual
;   behavior, show a short message and wait for the same key to be pressed
;   again.
;
; `zuninit` installs these and stays resident; `zuninit -R` (or /R) removes a
; resident copy again.

		.286
		.model tiny
		locals

_TEXT		segment use16
		assume cs:_TEXT
		org 100h
		assume es:nothing, ss:nothing, ds:_TEXT

		public start
start		proc near
		jmp	main
start		endp

; Resident part
; -------------

; INT 59h handler. AX indexes [error_messages] without a bounds check.
error_show	proc far
		pushf
		push	ax
		push	bx
		push	cx
		push	dx
		push	ds
		push	di
		push	es
		mov	bx, ax
		shl	bx, 1
		xor	ax, ax
		mov	dx, cs:error_messages[bx]
		call	text_put_sjis
		mov	ax, (1 * 80 * 2)
		mov	dx, offset aPRESS_RETURN
		call	text_put_sjis

@@wait_for_return:
		mov	ah, 4	; Keyboard: Read key state group
		mov	al, 3	; (Return is bit 4)
		int	18h
		test	ah, 10h
		jz	short @@wait_for_return
		pop	es
		pop	di
		pop	ds
		pop	dx
		pop	cx
		pop	bx
		pop	ax
		popf
		iret
error_show	endp

; INT 06h handler (STOP key)
stop_key_handler	proc far
		cmp	cs:key_handler_active, 0
		jnz	short @@ret
		mov	cs:key_handler_active, 1
		call	key_warning_show

@@ret:
		iret
stop_key_handler	endp

; INT 05h handler (COPY key)
copy_key_handler	proc far
		cmp	cs:key_handler_active, 0
		jnz	short @@ret
		mov	cs:key_handler_active, 2
		call	key_warning_show

@@ret:
		iret
copy_key_handler	endp

; Shows the message for the key in [key_handler_active] (1 = STOP, 2 = COPY)
; on text rows 10-12 from column 8, waits until that key is released and
; pressed again, and clears the message.
key_warning_show	proc near
		pushf
		push	ax
		push	bx
		push	cx
		push	dx
		push	ds
		push	di
		push	es
		mov	ah, 41h	; Hide graphics
		int	18h
		cmp	cs:key_handler_active, 2
		jz	short @@copy
		mov	ax, (((10 * 80) + 8) * 2)
		mov	dx, offset aSTOP_1
		call	text_put_sjis
		mov	ax, (((11 * 80) + 8) * 2)
		mov	dx, offset aSTOP_2
		call	text_put_sjis
		mov	ax, (((12 * 80) + 8) * 2)
		mov	dx, offset aSTOP_3
		call	text_put_sjis
		mov	bl, 1	; STOP is bit 0 of key state group 0Ch
		jmp	short @@wait_for_release
; ---------------------------------------------------------------------------

@@copy:
		mov	ax, (((10 * 80) + 8) * 2)
		mov	dx, offset aCOPY_1
		call	text_put_sjis
		mov	ax, (((11 * 80) + 8) * 2)
		mov	dx, offset aCOPY_2
		call	text_put_sjis
		mov	ax, (((12 * 80) + 8) * 2)
		mov	dx, offset aCOPY_3
		call	text_put_sjis
		mov	bl, 2	; COPY is bit 1 of key state group 0Ch

@@wait_for_release:
		mov	ah, 4
		mov	al, 0Ch
		int	18h
		test	ah, bl
		jnz	short @@wait_for_release

@@wait_for_press:
		mov	ah, 4
		mov	al, 0Ch
		int	18h
		test	ah, bl
		jz	short @@wait_for_press
		mov	ah, 40h	; Show graphics
		int	18h
		mov	ax, (((10 * 80) + 8) * 2)
		mov	dx, offset aBLANK
		call	text_put_sjis
		mov	ax, (((11 * 80) + 8) * 2)
		mov	dx, offset aBLANK
		call	text_put_sjis
		mov	ax, (((12 * 80) + 8) * 2)
		mov	dx, offset aBLANK
		call	text_put_sjis
		mov	ah, 6	; Keyboard: Initialize interface (flush buffer)
		int	18h
		pop	es
		pop	di
		pop	ds
		pop	dx
		pop	cx
		pop	bx
		pop	ax
		popf
		mov	cs:key_handler_active, 0
		retn
key_warning_show	endp

; Converts the Shift-JIS code point in AX (lead byte in AH) to JIS X 0208,
; with both bytes in the 21h-7Eh range.
sjis_to_jis	proc near
		shl	ah, 1
		cmp	al, 9Fh
		jnb	short @@second_half
		cmp	al, 80h
		adc	ax, 0FEDFh

@@second_half:
		sbb	ax, 0DFFEh
		and	ax, 7F7Fh
		retn
sjis_to_jis	endp

; Writes the '$'-terminated, full-width-only Shift-JIS string at CS:DX to
; text RAM at byte offset AX, and sets each written cell's attribute to 41h
; (yellow).
text_put_sjis	proc near
		mov	bx, dx
		mov	di, 0A000h	; Text RAM, characters
		mov	es, di
		mov	di, ax
		mov	dx, ax
		xor	ax, ax
		xor	cx, cx

@@char_loop:
		mov	ax, cs:[bx]
		cmp	al, '$'
		jz	short @@set_attributes
		xchg	ah, al
		call	sjis_to_jis
		xchg	ah, al
		sub	al, 20h
		stosw	; Left half…
		or	ah, 80h
		stosw	; …and right half of the full-width character
		inc	bx
		inc	bx
		inc	cx
		inc	cx
		jmp	short @@char_loop
; ---------------------------------------------------------------------------

@@set_attributes:
		mov	di, 0A200h	; Text RAM, attributes
		mov	es, di
		mov	di, dx
		mov	ax, 41h

@@attribute_loop:
		dec	cx
		stosw
		jz	short @@ret
		jmp	short @@attribute_loop
; ---------------------------------------------------------------------------

@@ret:
		retn
text_put_sjis	endp

; ---------------------------------------------------------------------------
; Identifies a resident copy, at the same offset in the INT 59h vector's
; segment.
ZUNP		dw 'ZU', 'NP'
int59_old	dd 0
int05_old	dd 0
int06_old	dd 0
key_handler_active	db 0
aPRESS_RETURN	db 'リターンキーを押してね$'
aERROR_GENERIC	db 'なんかのエラーですぅ$'
aERROR_FILE	db 'ファイルがみつかんないよぉ$'
aERROR_MEMORY	db 'メモリが足りないんですぅ$'
aERROR_BATCH	db 'バッチファイルから起動してよぉ$'
aERROR_SCOREDAT	db 'ハイスコアのファイルがおかしいよぅ、だからハイスコアは消えちゃったよ$'
error_messages	dw offset aERROR_GENERIC
		dw offset aERROR_GENERIC
		dw offset aERROR_FILE
		dw offset aERROR_MEMORY
		dw offset aERROR_BATCH
		dw offset aERROR_SCOREDAT
aSTOP_1		db 'むやみにＳＴＯＰキー押したりしない$'
aSTOP_2		db '方がいいと思うの。（ゲーム中はね）$'
aSTOP_3		db '（ＳＴＯＰキーで戻れるよ、ねっ）　$'
aBLANK		db '　　　　　　　　　　　　　　　　　$'
aCOPY_1		db 'なんでＣＯＰＹキー押したりしてるの$'
aCOPY_2		db 'かな～。ふしぎ～。　　（もう一度、$'
aCOPY_3		db 'ＣＯＰＹキー押せば戻れるよ、ねっ）$'

; Transient part
; --------------

; Prints the version banner. Returns 1 in AX if a copy of ZUNINIT is already
; resident, 0 if not. Also returns the current INT 59h vector in ES:BX.
; Its offset marks the end of the resident part.
resident_check	proc near
		mov	dx, offset aBANNER
		mov	ah, 9
		int	21h
		mov	ax, 3559h
		int	21h
		cmp	es:ZUNP, 'ZU'
		jnz	short @@not_resident
		cmp	es:ZUNP+2, 'NP'
		jnz	short @@not_resident
		mov	ax, 1
		jmp	short @@ret
; ---------------------------------------------------------------------------

@@not_resident:
		mov	ax, 0

@@ret:
		retn
resident_check	endp

main		proc near
		mov	si, 81h	; Command line in the PSP

@@skip_whitespace:
		lodsb
		cmp	al, 0Dh
		jz	short @@no_option
		cmp	al, '/'
		jz	short @@option
		cmp	al, '-'
		jz	short @@option
		cmp	al, ' '
		jbe	short @@skip_whitespace

@@no_option:
		call	resident_check
		test	ax, ax
		jz	short @@install
		jmp	@@already_resident
; ---------------------------------------------------------------------------

@@option:
		lodsb
		cmp	al, ' '
		ja	short @@option_letter
		jmp	@@invalid_option
; ---------------------------------------------------------------------------

@@option_letter:
		and	al, 0DFh	; to uppercase
		cmp	al, 'R'
		jz	short @@remove
		jmp	@@invalid_option
; ---------------------------------------------------------------------------

@@remove:
		call	resident_check
		test	ax, ax
		jnz	short @@uninstall
		jmp	@@not_resident
; ---------------------------------------------------------------------------

@@install:
		mov	word ptr cs:int59_old, bx
		mov	word ptr cs:int59_old+2, es
		mov	dx, offset error_show
		mov	ax, 2559h
		int	21h
		mov	ah, 35h
		mov	al, 6
		int	21h
		mov	word ptr cs:int06_old, bx
		mov	word ptr cs:int06_old+2, es
		mov	dx, offset stop_key_handler
		mov	ax, 2506h
		int	21h
		mov	ah, 35h
		mov	al, 5
		int	21h
		mov	word ptr cs:int05_old, bx
		mov	word ptr cs:int05_old+2, es
		mov	dx, offset copy_key_handler
		mov	ax, 2505h
		int	21h
		mov	dx, offset aINSTALLED
		mov	ah, 9
		int	21h
		mov	dx, offset resident_check
		mov	cl, 4
		shr	dx, cl
		inc	dx
		mov	ax, 3100h	; Terminate and stay resident
		int	21h
; ---------------------------------------------------------------------------

; ES = segment of the resident copy
@@uninstall:
		push	ds
		mov	dx, word ptr es:int59_old
		mov	ds, word ptr es:int59_old+2
		mov	ax, 2559h
		int	21h
		mov	dx, word ptr es:int06_old
		mov	ds, word ptr es:int06_old+2
		mov	ax, 2506h
		int	21h
		mov	dx, word ptr es:int05_old
		mov	ds, word ptr es:int05_old+2
		mov	ax, 2505h
		int	21h
		pop	ds
		push	es
		mov	es, word ptr es:2Ch	; Environment segment
		mov	ah, 49h
		int	21h
		pop	es
		mov	ah, 49h
		int	21h
		mov	dx, offset aREMOVED
		jnb	short @@print_removal_result
		mov	dx, offset aREMOVAL_FAILED

@@print_removal_result:
		mov	ah, 9
		int	21h
		jmp	short @@exit
; ---------------------------------------------------------------------------

@@already_resident:
		mov	dx, offset aALREADY_RESIDENT
		mov	ah, 9
		int	21h
		jmp	short @@exit
; ---------------------------------------------------------------------------

@@not_resident:
		mov	dx, offset aNOT_RESIDENT
		mov	ah, 9
		int	21h
		jmp	short @@exit
; ---------------------------------------------------------------------------

@@invalid_option:
		mov	dx, offset aINVALID_OPTION
		mov	ah, 9
		int	21h
		jmp	short $+2

@@exit:
		mov	ax, 4C00h
		int	21h
main		endp

; ---------------------------------------------------------------------------
aBANNER	db 0Dh,0Ah
		db 0Dh,0Ah
		db 'INTvector set program  zuninit.com Version1.00              (c)zun 1997',0Dh,0Ah,'$'
aINSTALLED	db 'ちょこっとメモリかりるね',0Dh,0Ah
		db 0Dh,0Ah,'$'
aALREADY_RESIDENT	db 'すでに常駐してるの',0Dh,0Ah
		db 0Dh,0Ah,'$'
aREMOVED	db 'メモリから消えてなくなっちゃったけど、きっとまたあえるよ、ねっ',0Dh,0Ah
		db 0Dh,0Ah,'$'
aNOT_RESIDENT	db 'まだ、常駐してないわぁ',0Dh,0Ah
		db 0Dh,0Ah,'$'
aINVALID_OPTION	db '意味不明なオプションよぉ（オプションは -Rのみ）',0Dh,0Ah,'$'
aREMOVAL_FAILED	db 'メモリ解放エラーです。 : zuninit.com',0Dh,0Ah,'$'
_TEXT		ends

		end start
