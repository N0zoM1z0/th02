; ReC98
; -----
; ZUN's hand-written pointnum_put() between the pointnum and shot TUs, as its
; own module so that all of main_01 can share one code segment.

	.386
	locals

BYTE_DOTS = 8
RES_X = 640
RES_Y = 400
ROW_SIZE = (RES_X / BYTE_DOTS)
PLANE_SIZE = (ROW_SIZE * RES_Y)
POINTNUM_H = 8

_DATA	segment word public 'DATA' use16
include th02/sprites/pointnum.asp
_DATA	ends

DGROUP group _DATA

main_01 group main_01_TEXT
main_01_TEXT	segment	word public 'CODE' use16
	assume cs:main_01, ds:DGROUP

include th02/main/pointnum/num_put.asm

main_01_TEXT	ends

	end
