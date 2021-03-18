;+==========================================================================-;
; main.asm: Entry point
;+==========================================================================-;
	.include	"macro.inc"
	.include	"system.inc"
	.include	"variables.inc"

	.org	$e000
	.include	"graphics.asm"
	
init
	sei	; disable all interrupts
	csh	; run at 7 mhz
	cld	; disable bcd
	lda	#$ff
	tax
	txs	; init stack pointer
	tam0	; i/o in bank 0
	lda	#$f8
	tam1	; ram in bank 1

;	Video init
	jsr	vdc_init
	lda	#%00000100	; low-res, sharper video
	sta	vce_ctrl

;	Enable interrupts
	stz	irq_dis
	cli

;	Clear SATB buffer
	stz	satb_buf
	tii	satb_buf, satb_buf + 1, (satb_buf_end - satb_buf) - 1

	stz	vce_adrl
	stz	vce_adrh
	tia	bg_pal, vce_datl, (bg_pal_end - bg_pal)
	
	stz	vce_adrl
	lda	#$1
	sta	vce_adrh
	tia	bg_pal, vce_datl, (bg_pal_end - bg_pal)
	
	set_reg(mawr)
	st1	#0
	st2	#0
	set_reg(vwr)
	lda	#$00
	sta	vdc_lo
	lda	#$1
	sta	vdc_hi
	lda	#$01
	sta	vdc_lo
	lda	#$1
	sta	vdc_hi
	lda	#$02
	sta	vdc_lo
	lda	#$1
	sta	vdc_hi
	lda	#$03
	sta	vdc_lo
	lda	#$1
	sta	vdc_hi
	
	set_reg(mawr)
	st1	#LO_BYTE(TILE_VRAM)
	st2	#HI_BYTE(TILE_VRAM)
	set_reg(vwr)
	; tia	bg_tile, vdc_lo, (bg_tile_end - bg_tile)
	tia	catboy_tile, vdc_lo, (catboy_tile_end - catboy_tile)
	

loop
	bra	loop


;	Init VDC from table
vdc_init
	clx
_init_loop
	lda	vdc_table, x
	bmi	_done_init	; branch when we hit the $ff at end of table
	sta	vdc_ar		; register number
	inx
	lda	vdc_table, x
	sta	vdc_lo		; low byte of register
	inx
	lda	vdc_table, x
	sta	vdc_hi		; high byte of register
	inx
	bra	_init_loop
_done_init
	rts


;	VDC initial register table
vdc_table
	.byte	cr	; Control register
			; Enable BG, sprites, vblank interrupt
	.word	%0000000011001000
	.byte	rcr	; Raster control register
	.word	$0
	.byte	bxr	; BG X scroll
	.word	$0
	.byte	byr	; BG Y scroll
	.word	$0
	.byte	mwr	; Memory access width register
	.word	$50	; 64x64 tile BG
	.byte	hsr	; Horizontal sync register
	.word	$0202	; 256x224 display
	.byte	hdr	; Horizontal display register
	.word	$031f	; 256x224 display
	.byte	vpr	; Vertical sync register
	.word	$1302	; 256x224 display
	.byte	vdr	; Vertical display register
	.word	$00df	; 256x224 display
	.byte	vcr	; Vertical display end pos register
	.word	$3	; 256x224 display
	.byte	dcr	; DMA control register
	.word	$10	; DMA VRAM to SATB each vblank
	.byte	dvssr	; DMA VRAM - SATB source register
	.word	SATB_VRAM	; SATB at 7f00 in vram
	.byte	$ff	; End of table

	
no_int
	rti

	
vdc_int
;	save state
	pha
	phx
	phy
;
	lda	vdc_ar	; acknowledge interrupt
	inc	frame_count
	
;	copy satb buffer to vram
	st0	#mawr	; set up address to copy
	st1	#LO_BYTE(SATB_VRAM)
	st2	#HI_BYTE(SATB_VRAM)
	st0	#vwr	; write to vram
	tia	satb_buf, vdc_lo, (satb_buf_end - satb_buf)

;	reset VDC address register
	lda	ar_buf
	sta	vdc_ar
;
;	restore state
	ply
	plx
	pla
	rti
	
	.org	$fff6
vectors
	.word	no_int	; IRQ2 vector (BRK)
	.word	vdc_int	; IRQ1 vector (VDC)
	.word	no_int	; Timer vector
	.word	no_int	; NMI vector (unused)
	.word	init	; Reset vector
	
	.end
