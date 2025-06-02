;;;
;;; regs
;;;
ARG	equ reg3
RET	equ reg3
SPH	equ reg4
SPL	equ reg5
RAH	equ reg6
RAL	equ reg7

;;;
;;; constants
;;;
NUL	equ 000
EOT	equ 004			; ctrl-d
CR	equ 00D
LF	equ 00A
BS	equ 008
DEL	equ 07F

;;;
;;; config
;;;
WORKB	equ 00400,adr		; WORK begin
STACK	equ 00600,adr		; Stack end

BUFLEN	equ 16			; Buffer length (16 or above)

;;;
;;; work area
;;;
INBUF	equ WORKB		; Line input buffer
DSADDR  equ INBUF+BUFLEN	; Dump start address
DEADDR	equ DSADDR+2		; Dump end address
ASCBUF	equ DEADDR+2		; Buffer for ASCII dump
DSTATE	equ ASCBUF+16		; Dump state
GADDR	equ DSTATE+1		; Go address
SADDR	equ GADDR+2		; Set address
HEXMOD	equ SADDR+2		; HEX file mode
RECTYP	equ HEXMOD+1		; Record type
PSPEC	equ RECTYP+1		; Processor spec.

;	IF USE_REGCMD
;REGA:	RMB	1		; Accumulator A
;REGB:	RMB	1		; Accumulator B
;REGE:	RMB	1		; Accumulator E (HD6309 only)
;REGF:	RMB	1		; Accumulator F (HD6309 only)
;REGX:	RMB	2		; Index register X
;REGY:	RMB	2		; Index register Y
;REGU:	RMB	2		; User stack pointer U
;REGS:	RMB	2		; System stack pointer S
;REGPC:	RMB	2		; Program counter PC
;REGV:	RMB	2		; Register V (HD6309 only)
;REGDP:	RMB	1		; Direct page register DP
;REGCC:	RMB	1		; Condition code register CC
;	ENDIF			; USE_REGCMD
;REGMD	RMB	1		; MD register

DMPPT	equ PSPEC+1
CKSUM	equ DMPPT+2		; Checksum


	org 0,0
;;;
;;; main
;;;
CSTART:	SPH=STACK>>8
	SPL=STACK&0FF

	adH=DSADDR>>8
	adL=DSADDR&0FF
	mem=0
	adL,psw=inc(adL)
	adH=incc(adH)
	mem=0

	adH=SADDR>>8
	adL=SADDR&0FF
	mem=0
	adL,psw=inc(adL)
	adH=incc(adH)
	mem=0

	adH=GADDR>>8
	adL=GADDR&0FF
	mem=0
	adL,psw=inc(adL)
	adH=incc(adH)
	mem=0

	adH=HEXMOD>>8
	adL=HEXMOD&0FF
	mem='S'

	adH=PSPEC>>8
	adL=PSPEC&0FF
	mem=0

; Opening message
	RAH=page(WSTART)
	RAL=WSTART
	acc=page(OPNMSG)
	goto acc,OPNMSG

WSTART:	RAH=page(LIN)
	RAL=LIN
	acc=page(PROMPT)
	goto acc,PROMPT
LIN:	RAH=page(LINE)
	RAL=LINE
	acc=page(GETLIN)
	goto acc,GETLIN
LINE:	adH=INBUF>>8
	adL=INBUF&0FF
	RAH=page(SKIPE)
	RAL=SKIPE
	acc=page(SKIPSP)
	goto acc,SKIPSP
SKIPE:	RAH=page(UPPE)
	RAL=UPPE
	acc=page(UPPER)
	goto acc,UPPER
UPPE:	acc,psw=RET
	if z goto WSTART
	reg0='D'
	psw,psw=sub(reg0)
	ARG=acc
	acc=page(DUMP)
	if z goto acc,DUMP	; DUMP

ERR:	RAH=page(ERRE)
	RAL=ERRE
	acc=page(ERRMSG)
	goto acc,ERRMSG
ERRE:	goto WSTART


;;;
;;; Strings
;;;
OPNMSG:	sio=CR
	sio=LF
	sio='U'
	sio='n'
	sio='i'
	sio='v'
	sio='e'
	sio='r'
	sio='s'
	sio='a'
	sio='l'
	sio=' '
	sio='M'
	sio='o'
	sio='n'
	sio='i'
	sio='t'
	sio='o'
	sio='r'
	sio=' '
	sio='Z'
	sio='E'
	sio='R'
	sio='O'
	sio=CR
	sio=LF
	;NUL
	goto RAH,RAL
PROMPT:	sio=']'
	sio=' '
	;NUL
	goto RAH,RAL
;IHEMSG:	DC "Error ihex",CR,LF,NUL
;SHEMSG:	DC "Error srec",CR,LF,NUL
ERRMSG:	sio='E'
	sio='r'
	sio='r'
	sio='o'
	sio='r'
	sio=CR
	sio=LF
	;NUL
	goto RAH,RAL
;DSEP0:	DC " :",NUL
;DSEP1:	DC " : ",NUL
;IHEXER:	DC ":00000001FF",CR,LF,NUL
;SRECER:	DC "S9030000FC",CR,LF,NUL


	org 001,0
;;;
;;; Dump memory
;;;
DUMP:	adL,psw=inc(adL)
	adH=incc(adH)
	RAH=page(SKPE2)
	RAL=SKPE2
	acc=page(SKIPSP)
	goto acc,SKIPSP
SKPE2:	RAH=page(RDHE1)
	RAL=RDHE1
	acc=page(RDHEX)
	goto acc,RDHEX
RDHE1:	psw,psw=pio		; count
	if !z goto DP0

; No arg.
	RAH=page(SKPE3)
	RAL=SKPE3
	acc=page(SKIPSP)
	goto acc,SKIPSP
SKPE3:	psw,psw=RET
	acc=page(ERR)
	if !z goto acc,ERR
NOARG:	reg0=adH		; index
	reg1=adL
	acc=128			; default end
	adH=page(DSADDR)
	adL=DSADDR
	adL,psw=inc(adL)
	adH=incc(adH)
	reg3,psw=add(mem)
	acc=0
	acc=incc(acc)
	adH=page(DSADDR)
	adL=DSADDR
	reg2=add(mem)
	adH=page(DEADDR)
	adL=DEADDR
	mem=reg2
	adL,psw=inc(adL)
	adH=incc(adH)
	mem=reg3
	goto DPM

; found 1st arg.
DP0:	reg0=adH		; index
	reg1=adL

	adH=page(DSADDR)
	adL=DSADDR
	mem=reg2
	adL,psw=inc(adL)
	adH=incc(adH)
	mem=reg3

	adH=reg0		; index
	adL=reg1

	RAH=page(SKPE4)
	RAL=SKPE4
	acc=page(SKIPSP)
	goto acc,SKIPSP
SKPE4:	acc=RET
	reg0=','
	psw,psw=sub(reg0)
	if z goto DP1
	psw,psw=RET
	acc=page(ERR)
	if !z goto acc,ERR
; No 2nd arg.
	goto NOARG

; found 2nd arg.
DP1:	adL,psw=inc(adL)
	adH=incc(adH)
	RAH=page(SKPE5)
	RAL=SKPE5
	acc=page(SKIPSP)
	goto acc,SKIPSP
SKPE5:	RAH=page(RDHE2)
	RAL=RDHE2
	acc=page(RDHEX)
	goto acc,RDHEX
RDHE2:	psw,psw=pio		; count
	acc=page(ERR)
	if z goto acc,ERR
	reg0=adH		; index
	reg1=adL
	reg3,psw=inc(reg3)
	reg2=incc(reg2)
	adH=page(DEADDR)
	adL=DEADDR
	mem=reg2
	adL,psw=inc(adL)
	adH=incc(adH)
	mem=reg3

; DUMP main
;
; reg2,3: current addr.
DPM:	adH=page(DSADDR)	; restore start addr.
	adL=DSADDR
	reg2=mem
	adL,psw=inc(adL)
	adH=incc(adH)
	acc=0F0
	reg3=and(mem)
	adH=page(DSTATE)
	adL=DSTATE
	mem=0
DPM0:	RAH=page(DPLE)
	RAL=DPLE
	acc=page(DPL)
	goto acc,DPL
; TODO: if SI goto DPM1
DPLE:	adH=page(DSTATE)
	adL=DSTATE
	acc=2
	psw,psw=sub(mem)
	if !z goto DPM0		; if state < 2 loop
	adH=page(DEADDR)	; save end addr. -> start addr.
	adL=DEADDR
	reg0=mem
	adL,psw=inc(adL)
	adH=incc(adH)
	reg1=mem
	adH=page(DSADDR)
	adL=DSADDR
	mem=reg0
	adL,psw=inc(adL)
	adH=incc(adH)
	mem=reg1
	acc=page(WSTART)
	goto acc,WSTART		; return
;DPM1:	adH=page(DSADDR)	; save start addr.
;	adL=DSADDR
;	mem=reg2
;	adL,psw=inc(adL)
;	adH=incc(adH)
;	mem=reg3
;	; TODO: SI
;	acc=page(WSTART)
;	goto acc,WSTART		; return

	org 002,0
; Dump line
DPL:	adH=SPH			; push RAL,RAH
	adL=SPL
	adL,psw=dec(adL)
	adH=decc(adH)
	mem=RAL
	adL,psw=dec(adL)
	adH=decc(adH)
	mem=RAH
	SPH=adH			; SP-2
	SPL=adL

	RAH=page(HOUT4E)
	RAL=HOUT4E
	acc=page(HEXOUT4)
	goto acc,HEXOUT4	; print addr.
HOUT4E:	sio=' '
	sio=':'
	adH=page(DSTATE)
	adL=DSTATE
	reg0=mem
	reg1=16
DPL0:	RAH=page(DPL0E)
	RAL=DPL0E
	acc=page(DPB)
	goto acc,DPB		; print byte
DPL0E:	reg1,psw=dec(reg1)
	if !z goto DPL0
	adH=page(DSTATE)
	adL=DSTATE
	mem=reg0
	sio=' '
	sio=':'
	sio=' '

; Print ASCII area
	adH=page(ASCBUF)
	adL=ASCBUF
	reg1=16
DPL1:	acc=mem
	adL,psw=inc(adL)
	adH=incc(adH)
	reg0=' '
	psw,psw=sub(reg0)
	if c goto DPL2
	reg0=07F
	psw,psw=sub(reg0)
	if !c goto DPL2
	sio=acc
	goto DPL3
DPL2:	sio='.'
DPL3:	reg1,psw=dec(reg1)
	if !z goto DPL1
	sio=CR
	sio=LF

	adH=SPH			; pop RAH,RAL
	adL=SPL
	RAH=mem
	adL,psw=inc(adL)
	adH=incc(adH)
	RAL=mem
	adL,psw=inc(adL)
	adH=incc(adH)
	SPH=adH			; SP+2
	SPL=adL
	goto RAH,RAL		; return

; Dump byte
;
; reg0:   state (0:skip head, 1:dump, 2:skip tail)
; reg1:   remain
; reg2,3: addr. H,L
DPB:	sio=' '
	psw,psw=reg0
	if !z goto DPB2		; if state==1 or 2 goto DPB2

; Dump state 0
	adH=page(DSADDR)
	adL=DSADDR
	adL,psw=inc(adL)
	adH=incc(adH)
	acc=mem			; L addr only
	psw,psw=sub(reg3)
	if z goto DPB1		; start dump
; Dump state 0 or 2
DPB0:	sio=' '			; skip
	sio=' '
	adH=page(ASCBUF)
	adL=ASCBUF
	acc=16
	acc=sub(reg1)		; pos
	adL,psw=add(adL)
	adH=incc(adH)
	mem=' '
	reg3,psw=inc(reg3)
	reg2=incc(reg2)
	goto RAH,RAL		; return

DPB1:	reg0=1			; state <- 1
DPB2:	psw,psw=dec(reg0)	; state 1 or 2
	if !z goto DPB0		; if state==2 goto DPB0

; Dump state 1
	adH=reg2
	adL=reg3
	reg0=mem
	reg3,psw=inc(reg3)
	reg2=incc(reg2)
	adH=page(ASCBUF)
	adL=ASCBUF
	acc=16
	acc=sub(reg1)		; pos
	adL,psw=add(adL)
	adH=incc(adH)
	mem=reg0		; src -> dst

	adH=SPH			; save regs
	adL=SPL
	adL,psw=dec(adL)
	adH=decc(adH)
	mem=reg2
	adL,psw=dec(adL)
	adH=decc(adH)
	mem=reg1
	adL,psw=dec(adL)
	adH=decc(adH)
	mem=RAL
	adL,psw=dec(adL)
	adH=decc(adH)
	mem=RAH
	SPH=adH			; SP-4
	SPL=adL
	RAH=page(HEXE)
	RAL=HEXE
	reg2=reg0
	acc=page(HEXOUT2)
	goto acc,HEXOUT2	; call (print hex)
	org 003,0
HEXE:	adH=SPH			; restore regs
	adL=SPL
	RAH=mem
	adL,psw=inc(adL)
	adH=incc(adH)
	RAL=mem
	adL,psw=inc(adL)
	adH=incc(adH)
	reg1=mem
	adL,psw=inc(adL)
	adH=incc(adH)
	reg2=mem
	adL,psw=inc(adL)
	adH=incc(adH)
	SPH=adH			; SP+4
	SPL=adL

	reg0=1			; state <- 1
	adH=page(DEADDR)
	adL=DEADDR
	acc=mem
	psw,psw=sub(reg2)
	if !z goto DPBE
	adL,psw=inc(adL)
	adH=incc(adH)
	acc=mem
	psw,psw=sub(reg3)
	if !z goto DPBE
	reg0=2			; state <- 2
DPBE:	goto RAH,RAL		; return


	org 0FE,0
;;;
;;; utils 1
;;;
RDHEX:	reg0=adH		; index
	reg1=adL

	adH=SPH			; push RAL,RAH
	adL=SPL
	adL,psw=dec(adL)
	adH=decc(adH)
	mem=RAL
	adL,psw=dec(adL)
	adH=decc(adH)
	mem=RAH
	SPH=adH			; SP-2
	SPL=adL

	adH=reg0		; index
	adL=reg1
	reg1=0			; result H
	reg2=0			; result L
	pio=0			; count
RH0:	ARG=mem
	RAH=page(RH0E)
	RAL=RH0E
	acc=page(UPPER)
	goto acc,UPPER
RH0E:	acc=RET
	reg0='0'
	psw,psw=sub(reg0)
	if c goto RHE
	reg0='9'+1
	psw,psw=sub(reg0)
	if c goto RH1
	reg0='A'
	psw,psw=sub(reg0)
	if c goto RHE
	reg0='F'+1
	psw,psw=sub(reg0)
	if !c goto RHE
	reg0='A'-'9'-1
	acc=sub(reg0)
RH1:	reg0='0'
	acc,psw=sub(reg0)	; clear carry
	reg2,psw=rl(reg2)
	reg1,psw=rl(reg1)
	reg2,psw=rl(reg2)
	reg1,psw=rl(reg1)
	reg2,psw=rl(reg2)
	reg1,psw=rl(reg1)
	reg2,psw=rl(reg2)
	reg1,psw=rl(reg1)
	reg2=or(reg2)		; reg12 = (reg12<<4) | acc
	adL,psw=inc(adL)
	adH=incc(adH)
	pio=inc(pio)
	goto RH0

RHE:	reg3=reg2		; result L
	reg2=reg1		; result H
	reg0=adH		; index
	reg1=adL

	adH=SPH			; pop RAH,RAL
	adL=SPL
	RAH=mem
	adL,psw=inc(adL)
	adH=incc(adH)
	RAL=mem
	adL,psw=inc(adL)
	adH=incc(adH)
	SPH=adH			; SP+2
	SPL=adL

	adH=reg0		; index
	adL=reg1
	goto RAH,RAL		; return

HEXOUT4:adH=SPH			; push RAL,RAH,reg2(H)
	adL=SPL
	adL,psw=dec(adL)
	adH=decc(adH)
	mem=RAL
	adL,psw=dec(adL)
	adH=decc(adH)
	mem=RAH
	adL,psw=dec(adL)
	adH=decc(adH)
	mem=reg2
	SPH=adH			; SP-3
	SPL=adL
; High
	RAH=page(H2E1)
	RAL=H2E1
	goto HEXOUT2
; Low
H2E1:	RAH=page(H2E2)
	RAL=H2E2
	reg2=reg3
	goto HEXOUT2

H2E2:	adH=SPH			; pop reg2,RAH,RAL
	adL=SPL
	reg2=mem
	adL,psw=inc(adL)
	adH=incc(adH)
	RAH=mem
	adL,psw=inc(adL)
	adH=incc(adH)
	RAL=mem
	adL,psw=inc(adL)
	adH=incc(adH)
	SPH=adH			; SP+3
	SPL=adL
	goto RAH,RAL		; return

HEXOUT2:reg1=2
H2S:	reg2=rrc(reg2)
	reg2=rrc(reg2)
	reg2=rrc(reg2)
	reg2=rrc(reg2)
	acc=reg2
	reg0=00F
	acc=and(reg0)
	reg0='0'
	acc=add(reg0)
	reg0='9'+1
	psw,psw=sub(reg0)
	if c goto HEXOUTE
	reg0='A'-'9'-1
	acc=add(reg0)
HEXOUTE:sio=acc
	reg1,psw=dec(reg1)
	if !z goto H2S
	goto RAH,RAL		; return


	org 0FF,0
;;;
;;; utils 2
;;;
GETLIN:	adH=SPH			; push RAL,RAH
	adL=SPL
	adL,psw=dec(adL)
	adH=decc(adH)
	mem=RAL
	adL,psw=dec(adL)
	adH=decc(adH)
	mem=RAH
	SPH=adH			; SP-2
	SPL=adL

	adH=INBUF>>8		; buffer
	adL=INBUF&0FF
	reg2=0			; reg2: len

	RAH=page(GL1)
	RAL=GL1
GL0:	goto CONIN
GL1:	reg1=acc		; reg1: char

	reg0=CR
	psw,psw=sub(reg0)
	if z goto GLE		; CR
	reg0=LF
	psw,psw=sub(reg0)
	if z goto GLE		; LF

	reg0=BS
	psw,psw=sub(reg0)
	if z goto GLB		; BS
	reg0=DEL
	psw,psw=sub(reg0)
	if z goto GLB		; DEL

	reg0=EOT
	psw,psw=sub(reg0)
	if z goto EXIT		; if ctrl-d goto exit

	acc=reg1		; reg1: char, reg2: len
	reg0=' '
	psw,psw=sub(reg0)
	if c goto GL0		; ignore char < ' '
	reg0=080
	psw,psw=sub(reg0)
	if !c goto GL0		; ignore char > DEL

	acc=reg2		; reg1: char, reg2: len
	reg0=BUFLEN-1
	psw,psw=sub(reg0)
	if !c goto GL0		; too long

	mem=reg1		; save
	adL,psw=inc(adL)
	adH=incc(adH)
	reg2=inc(reg2)
	sio=reg1		; echo
	goto GL0

GLB:	psw,psw=reg2		; reg2: len
	if z goto GL0
	reg2=dec(reg2)
	adL,psw=dec(adL)
	adH=decc(adH)
	sio=008
	sio=' '
	sio=008
	goto GL0

GLE:	sio=CR
	sio=LF
	mem=NUL
	reg0=reg2		; len

	adH=SPH			; pop RAH,RAL
	adL=SPL
	RAH=mem
	adL,psw=inc(adL)
	adH=incc(adH)
	RAL=mem
	adL,psw=inc(adL)
	adH=incc(adH)
	SPH=adH			; SP+2
	SPL=adL
	goto RAH,RAL		; return

SKIPSP:	reg0=' '
SSB:	acc=mem
	psw,psw=sub(reg0)
	if !z goto SSE
	adL,psw=inc(adL)
	adH=incc(adH)
	goto SSB
SSE:	RET=acc
	goto RAH,RAL		; return

UPPER:	reg0='a'
	acc=ARG
	psw,psw=sub(reg0)
	if c goto UPE
	reg0='z'+1
	psw,psw=sub(reg0)
	if !c goto UPE
	reg0='A'-'a'
	acc=add(reg0)		; 'a' <= acc <= 'z'
UPE:	RET=acc
	goto RAH,RAL		; return


;;;
;;; sio
;;;
CONIN:	psw,psw=psw
	if !i goto CONIN
CIN1:	psw,psw=psw
	if i goto CIN1
	acc=sio
	goto RAH,RAL		; return


	org 0FF,07F
;;;
;;; exit
;;;
EXIT:	goto $
