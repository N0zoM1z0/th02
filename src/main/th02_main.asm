; ReC98
; -----
; MAIN.EXE: master.lib's code and data, linked as assembly in the
; original order, and the declarations of the program's code segments.

		.386
		.model use16 large _TEXT

BINARY = 'M'

include ReC98.inc
include th02/th02.inc

	extern SCOPY@:proc
	extern _execl:proc
	extern _getdate:proc
	extern _memcpy:proc

main_01 group main_01_TEXT
main_03 group main_03_TEXT, BULLET_TEXT, DIALOG_TEXT, BOSS_5_TEXT, main_03__TEXT
main_06 group REGIST_M_TEXT, main_06_TEXT

_TEXT		segment	word public 'CODE' use16
		assume cs:_TEXT
		assume es:nothing, ds:_DATA, fs:nothing, gs:nothing

include libs/master.lib/bfnt_entry_pat.asm
include libs/master.lib/bfnt_extend_header_skip.asm
include libs/master.lib/bfnt_header_read.asm
include libs/master.lib/bfnt_header_analysis.asm
include libs/master.lib/bcloser.asm
include libs/master.lib/bfill.asm
include libs/master.lib/bfnt_palette_set.asm
include libs/master.lib/bgetc.asm
include libs/master.lib/palette_black_out.asm
include libs/master.lib/bopenr.asm
include libs/master.lib/bread.asm
include libs/master.lib/bseek.asm
include libs/master.lib/bseek_.asm
include libs/master.lib/cutline.asm
include libs/master.lib/dos_axdx.asm
include libs/master.lib/dos_filesize.asm
include libs/master.lib/dos_setvect.asm
include libs/master.lib/egc.asm
include libs/master.lib/file_append.asm
include libs/master.lib/file_close.asm
include libs/master.lib/file_exist.asm
include libs/master.lib/file_read.asm
include libs/master.lib/file_ropen.asm
include libs/master.lib/file_seek.asm
include libs/master.lib/file_size.asm
include libs/master.lib/file_write.asm
include libs/master.lib/dos_close.asm
include libs/master.lib/dos_ropen.asm
include libs/master.lib/grcg_boxfill.asm
include libs/master.lib/grcg_byteboxfill_x.asm
include libs/master.lib/grcg_circlefill.asm
include libs/master.lib/grcg_circle.asm
		db 0
include libs/master.lib/grcg_circle_x.asm
include libs/master.lib/grc_setclip.asm
include libs/master.lib/grcg_fill.asm
include libs/master.lib/grcg_hline.asm
include libs/master.lib/grcg_line.asm
include libs/master.lib/grcg_pset.asm
include libs/master.lib/grcg_setcolor.asm
include libs/master.lib/grcg_vline.asm
include libs/master.lib/gdc_outpw.asm
		db 0
include libs/master.lib/gaiji_backup.asm
include libs/master.lib/gaiji_entry_bfnt.asm
include libs/master.lib/gaiji_putca.asm
include libs/master.lib/gaiji_putsa.asm
include libs/master.lib/gaiji_read.asm
include libs/master.lib/gaiji_write.asm
include libs/master.lib/graph_400line.asm
include libs/master.lib/graph_clear.asm
include libs/master.lib/graph_copy_page.asm
include libs/master.lib/graph_extmode.asm
include libs/master.lib/graph_pi_free.asm
include libs/master.lib/graph_pi_load_pack.asm
include libs/master.lib/graph_pack_put_8.asm
include libs/master.lib/graph_scrollup.asm
include libs/master.lib/graph_scroll.asm
include libs/master.lib/iatan2.asm
include libs/master.lib/key_sense.asm
include libs/master.lib/palette_show.asm
include libs/master.lib/pfclose.asm
include libs/master.lib/pfgetc.asm
include libs/master.lib/pfread.asm
include libs/master.lib/pfrewind.asm
include libs/master.lib/pfseek.asm
include libs/master.lib/random.asm
include libs/master.lib/rottbl.asm
include libs/master.lib/smem_release.asm
include libs/master.lib/smem_wget.asm
include libs/master.lib/text_boxfilla.asm
include libs/master.lib/text_clear.asm
include libs/master.lib/text_fillca.asm
include libs/master.lib/text_putca.asm
include libs/master.lib/text_putsa.asm
include libs/master.lib/vsync.asm
include libs/master.lib/vsync_wait.asm
include libs/master.lib/palette_white_in.asm
include libs/master.lib/palette_white_out.asm
include libs/master.lib/hmem_lallocate.asm
include libs/master.lib/mem_assign_dos.asm
include libs/master.lib/mem_assign.asm
include libs/master.lib/memheap.asm
include libs/master.lib/mem_unassign.asm
include libs/master.lib/super_free.asm
include libs/master.lib/super_entry_pat.asm
include libs/master.lib/super_put_1plane.asm
include libs/master.lib/super_entry_at.asm
include libs/master.lib/super_entry_bfnt.asm
include libs/master.lib/super_cancel_pat.asm
include libs/master.lib/super_clean.asm
include libs/master.lib/super_roll_put_1plane.asm
include libs/master.lib/super_roll_put.asm
include libs/master.lib/super_put_rect.asm
		db 0
include libs/master.lib/super_put.asm
include libs/master.lib/super_roll_put_tiny.asm
include libs/master.lib/super_convert_tiny.asm
include libs/master.lib/super_zoom.asm

	public pferrno, pfkey
	public mem_AllocID
	extrn PFOPEN:far, PFSTART:far, PFEND:far

_TEXT		ends

main_01_TEXT	segment	word public 'CODE' use16
		assume cs:main_01
		assume es:nothing, ss:nothing, ds:_DATA, fs:nothing, gs:nothing

main_01_TEXT	ends

SHARED	segment	word public 'CODE' use16
include th02/snd/snd.inc
SHARED	ends

main_03_TEXT	segment	byte public 'CODE' use16
main_03_TEXT	ends

BULLET_TEXT	segment	byte public 'CODE' use16
		assume cs:main_03
		assume es:nothing, ss:nothing, ds:_DATA, fs:nothing, gs:nothing

BULLET_TEXT ends

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

REGIST_M_TEXT segment	byte public 'CODE' use16
REGIST_M_TEXT ends

main_06_TEXT	segment	byte public 'CODE' use16
main_06_TEXT	ends

	.data

include libs/master.lib/atan8[data].asm
include libs/master.lib/bfnt_id[data].asm
include libs/master.lib/clip[data].asm
include libs/master.lib/edges[data].asm
include libs/master.lib/fil[data].asm
include libs/master.lib/dos_ropen[data].asm
include libs/master.lib/gaiji_backup[data].asm
include libs/master.lib/gaiji_entry_bfnt[data].asm
include libs/master.lib/grp[data].asm
include libs/master.lib/pal[data].asm
include libs/master.lib/pf[data].asm
include libs/master.lib/rand[data].asm
include libs/master.lib/sin8[data].asm
include libs/master.lib/tx[data].asm
include libs/master.lib/version[data].asm
; libs/master.lib/vs[data].asm, with the original library's NOP for EVEN
	public _vsync_Proc,vsync_Proc
vsync_Proc label dword
_vsync_Proc	dd	0

	public _vsync_Delay,vsync_Delay
vsync_Delay label word
_vsync_Delay	dw	0

	public vsync_OldMask,_vsync_OldMask
_vsync_OldMask label byte
vsync_OldMask	db	0
	db	90h
include libs/master.lib/wordmask[data].asm
include libs/master.lib/mem[data].asm
include libs/master.lib/super_entry_bfnt[data].asm
include libs/master.lib/superpa[data].asm

	.data?

include libs/master.lib/clip[bss].asm
include libs/master.lib/fil[bss].asm
include libs/master.lib/grcg_circle[bss].asm
include libs/master.lib/pal[bss].asm
include libs/master.lib/vs[bss].asm
include libs/master.lib/vsync[bss].asm
include libs/master.lib/mem[bss].asm
include libs/master.lib/superpa[bss].asm
include libs/master.lib/super_put_rect[bss].asm

		end
