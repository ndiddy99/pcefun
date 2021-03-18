;+==========================================================================-;
; variables.inc: variables
;+==========================================================================-;
;	VRAM regions
TILE_VRAM	.equ	$1000
SATB_VRAM	.equ	$7f00


;	Zero page variables
		.org	$2000
	
ar_buf		.block	1	; buffer for vdc so interrupts don't mess us up
frame_count	.block	1	; incremented each vblank
_ax				; general purpose variables for parameters/scratchpad
_al		.block	1
_ah		.block	1
_bx
_bl		.block	1
_bh		.block	1
_cx
_cl		.block	1
_ch		.block	1
_dx
_dl		.block	1
_dh		.block	1

;	Macro that uses ar_buf to set vdc register number
#define	set_reg(reg_num)	lda	#reg_num
#defcont			\ sta	ar_buf
#defcont			\ st0	#reg_num
	
;	Regular variables
		.org	$2200
		
satb_buf	.block	512	; SATB buffer
satb_buf_end
