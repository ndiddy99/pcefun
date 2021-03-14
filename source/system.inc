.nolist
;+==========================================================================-;
; system.inc: PC Engine system equates
;+==========================================================================-;

; HuC6260 Video Color Encoder

vce_ctrl	.equ $0400
vce_adrl	.equ $0402
vce_adrh	.equ $0403
vce_datl	.equ $0404
vce_dath	.equ $0405

; HuC6270 Video Display Controller

vdc_ar		.equ $0000
vdc_lo		.equ $0002
vdc_hi		.equ $0003

mawr		.equ $00
marr		.equ $01
vwr		.equ $02
vrr		.equ $02
cr		.equ $05
rcr		.equ $06
bxr		.equ $07
byr		.equ $08
mwr		.equ $09
hsr		.equ $0a
hdr		.equ $0b
vpr		.equ $0c
vdr		.equ $0d
vcr		.equ $0e  
dcr		.equ $0f
sour		.equ $10
desr		.equ $11
lenr		.equ $12
dvssr		.equ $13 

; HuC6280 8-bit Microprocessor

; PSG ports

psg_r0		.equ $0800
psg_r1		.equ $0801
psg_r2		.equ $0802
psg_r3		.equ $0803
psg_r4		.equ $0804
psg_r5		.equ $0805
psg_r6		.equ $0806
psg_r7		.equ $0807
psg_r8		.equ $0808
psg_r9		.equ $0809    

; Port O: Joypad

joyio	  	.equ $1000
trg1	  	.equ 1
trg2	  	.equ 2
sel	  	.equ 4
run	  	.equ 8
up	  	.equ 16
right	  	.equ 32
down	  	.equ 64
left	  	.equ 128

; Interrupt registers
irq_reg		.equ $1402
irq_dis		.equ $1403

.list
