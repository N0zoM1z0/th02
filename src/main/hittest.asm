; ReC98
; -----
; TH02's shot hit testing, ZUN's own assembly module. In the original, its
; relocations are listed in ascending order (TASM), unlike every TCC object
; around it, and the module's code segment is word-aligned, which TCC never
; emits. The code itself is TCC output for the 80286 (inverted short jumps
; over near JMPs), with a 16-bit `add` into the 32-bit [score_delta] at the
; end.

	.386
	locals

_DATA	segment word public 'DATA' use16
	extrn _shot_level:byte
	extrn _page_back:byte
	extrn _bombing:byte
	extrn _stage_frame:dword
	extrn _score_delta:dword
	extrn _shots:byte
public _SHOT_DAMAGE, _OPTION_SHOT_DAMAGE, _SHOT_DAMAGE_BIG
_SHOT_DAMAGE	dw 5, 5, 4, 4, 3, 3, 3, 2, 2, 1
_OPTION_SHOT_DAMAGE	dw 2, 2, 2, 2, 2, 1, 1, 1, 1, 1
_SHOT_DAMAGE_BIG	dw 0, 0, 8, 8, 7, 7, 6, 5, 4, 3
		dw 0	; unused
public _byte_1EB88
_byte_1EB88	db 1
_DATA	ends

DGROUP group _DATA

	extrn @SPARKS_ADD$QIUIIII:far

main_03 group main_03_TEXT
main_03_TEXT	segment	word public 'CODE' use16
	assume cs:main_03, ds:DGROUP

public @SHOTS_HITTEST$QIIII
@SHOTS_HITTEST$QIIII	proc near

var_12		= word ptr -12h
var_10		= word ptr -10h
var_E		= word ptr -0Eh
var_B		= byte ptr -0Bh
var_A		= word ptr -0Ah
var_8		= word ptr -8
var_6		= word ptr -6
@@left		= word ptr -4
var_2		= word ptr -2
arg_0		= word ptr  4
arg_2		= word ptr  6
arg_4		= word ptr  8
arg_6		= word ptr  0Ah

		push	bp
		mov	bp, sp
		sub	sp, 12h
		push	si
		push	di
		mov	bl, _shot_level
		mov	bh, 0
		add	bx, bx
		mov	ax, _SHOT_DAMAGE[bx]
		mov	[bp+var_E], ax
		mov	ax, _OPTION_SHOT_DAMAGE[bx]
		mov	[bp+var_10], ax
		mov	ax, _SHOT_DAMAGE_BIG[bx]
		mov	[bp+var_12], ax
		mov	[bp+var_8], 0
		mov	ax, [bp+arg_6]
		add	ax, [bp+arg_2]
		mov	[bp+var_A], ax
		mov	al, _page_back
		shl	al, 2
		add	al, 4
		mov	[bp+var_B], al
		mov	ax, [bp+arg_4]
		add	ax, 0FFF8h
		mov	[bp+var_6], ax
		sub	[bp+arg_6], 8
		mov	si, offset _shots
		mov	[bp+var_2], 0

loc_1288F:
		cmp	byte ptr [si], 1
		jz	short loc_12897
		jmp	loc_129AB
; ---------------------------------------------------------------------------

loc_12897:
		cmp	byte ptr [si+0Fh], 0
		jz	short loc_128A0
		jmp	loc_12943
; ---------------------------------------------------------------------------

loc_128A0:
		cmp	byte ptr [si+1], 0
		jz	short loc_128A9
		jmp	loc_129AB
; ---------------------------------------------------------------------------

loc_128A9:
		mov	al, [bp+var_B]
		mov	ah, 0
		mov	bx, ax
		mov	ax, [bx+si-2]
		sar	ax, 4
		mov	[bp+@@left], ax
		cmp	ax, [bp+arg_6]
		jg	short loc_128C1
		jmp	loc_129AB
; ---------------------------------------------------------------------------

loc_128C1:
		cmp	ax, [bp+var_A]
		jl	short loc_128C9
		jmp	loc_129AB
; ---------------------------------------------------------------------------

loc_128C9:
		mov	al, [bp+var_B]
		mov	ah, 0
		mov	bx, ax
		mov	ax, [bx+si]
		sar	ax, 4
		mov	di, ax
		cmp	di, [bp+var_6]
		jg	short loc_128DF
		jmp	loc_129AB
; ---------------------------------------------------------------------------

loc_128DF:
		mov	ax, [bp+var_6]
		add	ax, [bp+arg_0]
		cmp	ax, di
		jg	short loc_128EC
		jmp	loc_129AB
; ---------------------------------------------------------------------------

loc_128EC:
		inc	[bp+var_8]
		cmp	word ptr [si+0Eh], 32h ; '2'
		jl	short loc_12926
		sar	word ptr [si+0Ah], 3
		sar	word ptr [si+0Ch], 3
		cmp	word ptr [si+0Eh], 78h ; 'x'
		jl	short loc_1291C
		mov	byte ptr [si+1], 1
		mov	ax, [bx+si-2]
		sub	ax, 80h
		mov	[bx+si-2], ax
		mov	ax, [bx+si]
		sub	ax, 80h
		mov	[bx+si], ax
		mov	ax, [bp+var_12]
		jmp	short loc_12923
; ---------------------------------------------------------------------------

loc_1291C:
		mov	byte ptr [si+1], 2
		mov	ax, [bp+var_E]

loc_12923:
		add	[bp+var_8], ax

loc_12926:
		push	[bp+@@left]	; left
		mov	al, [bp+var_B]
		mov	ah, 0
		mov	bx, ax
		mov	ax, [bx+si]
		sar	ax, 4
		push	ax	; top
		push	((1 shl 4) + 14)	; speed_base
		push	1	; count
		push	0	; as_sprite
		call	@SPARKS_ADD$QIUIIII
		jmp	short loc_129AB
; ---------------------------------------------------------------------------

loc_12943:
		mov	al, [bp+var_B]
		mov	ah, 0
		mov	bx, ax
		mov	ax, [bx+si-2]
		sar	ax, 4
		mov	[bp+@@left], ax
		cmp	ax, [bp+arg_6]
		jle	short loc_129AB
		mov	ax, [bp+arg_6]
		add	ax, [bp+arg_2]
		cmp	ax, [bp+@@left]
		jle	short loc_129AB
		mov	al, [bp+var_B]
		mov	ah, 0
		mov	bx, ax
		mov	ax, [bx+si]
		sar	ax, 4
		mov	di, ax
		cmp	di, [bp+var_6]
		jle	short loc_129AB
		mov	ax, [bp+var_6]
		add	ax, [bp+arg_0]
		cmp	ax, di
		jle	short loc_129AB
		cmp	byte ptr [si+1], 1
		jnz	short loc_129A5
		push	[bp+@@left]	; left
		mov	al, [bp+var_B]
		mov	ah, 0
		mov	bx, ax
		mov	ax, [bx+si]
		sar	ax, 4
		push	ax	; top
		push	((2 shl 4) + 8)	; speed_base
		push	1	; count
		push	0	; as_sprite
		call	@SPARKS_ADD$QIUIIII
		mov	byte ptr [si+1], 2

loc_129A5:
		mov	ax, [bp+var_10]
		add	[bp+var_8], ax

loc_129AB:
		inc	[bp+var_2]
		add	si, 10h
		cmp	[bp+var_2], 23h	; '#'
		jge	short loc_129BA
		jmp	loc_1288F
; ---------------------------------------------------------------------------

loc_129BA:
		cmp	_bombing, 0
		jz	short loc_129CD
		mov	al, _byte_1EB88
		test	byte ptr _stage_frame, al
		jnz	short loc_129CD
		inc	[bp+var_8]

loc_129CD:
		mov	ax, [bp+var_8]
		add	word ptr _score_delta, ax
		mov	ax, [bp+var_8]
		pop	di
		pop	si
		leave
		retn	8
@SHOTS_HITTEST$QIIII	endp
main_03_TEXT	ends

	end
