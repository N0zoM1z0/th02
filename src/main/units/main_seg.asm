; ReC98
; -----
; Declares MAIN.EXE's segments and groups in their original order, before any
; other object. This lets the first C++ TU of code segment #1 (with its own
; data) precede th02_main.asm in the link, as in the original, while code
; segment #1 still starts word-aligned.

	.386
	.model use16 large _TEXT

main_01 group main_01_TEXT
main_03 group main_03_TEXT, BULLET_TEXT, DIALOG_TEXT, BOSS_5_TEXT, main_03__TEXT
main_06 group REGIST_M_TEXT, main_06_TEXT

_TEXT	segment	word public 'CODE' use16
_TEXT	ends
main_01_TEXT	segment	word public 'CODE' use16
main_01_TEXT	ends
SHARED	segment	word public 'CODE' use16
SHARED	ends
main_03_TEXT	segment	byte public 'CODE' use16
main_03_TEXT	ends
BULLET_TEXT	segment	byte public 'CODE' use16
BULLET_TEXT	ends
DIALOG_TEXT	segment	byte public 'CODE' use16
DIALOG_TEXT	ends
BOSS_5_TEXT	segment	byte public 'CODE' use16
BOSS_5_TEXT	ends
main_03__TEXT	segment	byte public 'CODE' use16
main_03__TEXT	ends
main_04_TEXT	segment	byte public 'CODE' use16
main_04_TEXT	ends
main_05_TEXT	segment	byte public 'CODE' use16
main_05_TEXT	ends
REGIST_M_TEXT	segment	byte public 'CODE' use16
REGIST_M_TEXT	ends
main_06_TEXT	segment	byte public 'CODE' use16
main_06_TEXT	ends

	.data
	.data?

	end
