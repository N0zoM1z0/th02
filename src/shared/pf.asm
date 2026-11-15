; ReC98
; -----
; master.lib's pfint21 and pf_str_ieq around TH02's modified pfopen(), with
; their data, as one module shared by OP, MAIN and MAINE.

	.386
	.model use16 large _TEXT

include ReC98.inc

	.data
	extrn pferrno:word
	extrn pfkey:byte
	extrn mem_AllocID:word
include th02/formats/pfopen[data].asm

TEXT group _TEXT
_TEXT	segment	word public 'CODE' use16
	assume cs:TEXT
	assume es:nothing, ds:_DATA, fs:nothing, gs:nothing
	extrn BCLOSER:far
	extrn BOPENR:far
	extrn BREAD:far
	extrn BSEEK_:far
	extrn HMEM_ALLOCBYTE:far
	extrn HMEM_FREE:far
	extrn PFCLOSE:far
	extrn PFREAD:far
	extrn PFREWIND:far
	extrn PFSEEK:far
	extrn PFGETC1:near
	extrn PFGETX1:near

include libs/master.lib/pfint21.asm
	db 0
include th02/formats/pfopen.asm
include libs/master.lib/pf_str_ieq.asm

_TEXT	ends

	.data?
include libs/master.lib/pfint21[bss].asm

	end
