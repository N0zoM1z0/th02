; ReC98
; -----
; MAINE.EXE: master.lib's code and data, linked as assembly in the
; original order, and the declarations of the program's code segments.

		.386
		.model use16 large _TEXT

BINARY = 'E'

include ReC98.inc
include th02/th02.inc

	extern _execl:proc

maine_01 group END_TEXT, maine_01_TEXT

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
include libs/master.lib/palette_black_in.asm
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
include libs/master.lib/egc_shift_down.asm
include libs/master.lib/egc_shift_left.asm
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
include libs/master.lib/grcg_line.asm
include libs/master.lib/grcg_setcolor.asm
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
include libs/master.lib/graph_gaiji_putc.asm
include libs/master.lib/graph_gaiji_puts.asm
include libs/master.lib/graph_pi_free.asm
include libs/master.lib/graph_pi_load_pack.asm
include libs/master.lib/graph_pack_put_8.asm
include libs/master.lib/key_sense.asm
include libs/master.lib/over_put_8.asm
include libs/master.lib/palette_show.asm
include libs/master.lib/pfclose.asm
include libs/master.lib/pfgetc.asm
include libs/master.lib/pfread.asm
include libs/master.lib/pfrewind.asm
include libs/master.lib/pfseek.asm
include libs/master.lib/palette_entry_rgb.asm
include libs/master.lib/rottbl.asm
include libs/master.lib/smem_release.asm
include libs/master.lib/smem_wget.asm
include libs/master.lib/text_clear.asm
include libs/master.lib/vsync.asm
include libs/master.lib/vsync_wait.asm
include libs/master.lib/palette_white_in.asm
include libs/master.lib/palette_white_out.asm
include libs/master.lib/hmem_lallocate.asm
include libs/master.lib/mem_assign_dos.asm
include libs/master.lib/mem_assign.asm
include libs/master.lib/memheap.asm
include libs/master.lib/mem_unassign.asm
include libs/master.lib/super_entry_pat.asm
include libs/master.lib/super_entry_at.asm
include libs/master.lib/super_entry_bfnt.asm

	public pferrno, pfkey
	public mem_AllocID
	extrn PFOPEN:far, PFSTART:far, PFEND:far
_TEXT		ends

END_TEXT segment byte public 'CODE' use16
END_TEXT ends

maine_01_TEXT	segment	byte public 'CODE' use16
		assume cs:maine_01
		assume es:nothing, ss:nothing, ds:_DATA, fs:nothing, gs:nothing

maine_01_TEXT	ends

SHARED	segment	word public 'CODE' use16
SHARED	ends

maine_03_TEXT	segment	word public 'CODE' use16
maine_03_TEXT	ends

maine_04_TEXT	segment	byte public 'CODE' use16
maine_04_TEXT	ends

	.data

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
include libs/master.lib/mem[data].asm
include libs/master.lib/super_entry_bfnt[data].asm
include libs/master.lib/superpa[data].asm

	.data?

include libs/master.lib/clip[bss].asm
include libs/master.lib/fil[bss].asm
include libs/master.lib/pal[bss].asm
include libs/master.lib/vs[bss].asm
include libs/master.lib/vsync[bss].asm
include libs/master.lib/mem[bss].asm
include libs/master.lib/superpa[bss].asm
extern _resident:dword

		end
