;
; test
;
	; mem[adL++] <- 1
	acc=0FE			; 0xFE
	acc,psw=inc(acc)	; 0xFF, s(neg)
	acc,psw=inc(acc)	; 0x00, z, v, c
	acc,psw=inc(acc)	; 0x01
	mem=acc
	pio=mem			; 0x01 out
	adL,psw=inc(adL)

	; mem[adL++] <- 3 and 3
	acc,psw=inc(acc)	; 0x02
	acc,psw=inc(acc)	; 0x03
	mem=acc
	mem,psw=and(mem)	; mem: w -> r -> mod -> w -> r
	pio=mem			; 0x03 out
	adL,psw=inc(adL)
;
	adL=0
	acc=mem			; mem[0]: 0x01
	adL=1
	acc=mem			; mem[1]: 0x03

;
; sample: sum of 1..10
;

	reg0=((((((5+15-(-(-10)))*(+2)/4)<<2)>>1) & 0F) | ~(~02)) % 13
	acc=0
LOOP:	acc=add(reg0)
	reg0,psw=dec(reg0)	; z, n(sub)
	if !z goto LOOP
	pio=acc			; 0x37=55 out

;
; print
;

	sio=068			; 'h'
	sio=065			; 'e'
	sio=06C			; 'l'
	sio=06C			; 'l'
	sio=06F			; 'o'
	sio=02C			; ','
	sio=077			; 'w'
	sio=06F			; 'o'
	sio=072			; 'r'
	sio=06C			; 'l'
	sio=064			; 'd'

;
; echo
;
	reg0=004		; ctrl-d
	reg1=00D		; CR
	reg2=page(EXIT)

ECHO:	sio=00D			; '\r'
	sio=00A			; '\n'
	sio=03E			; '>'
	sio=020			; ' '

IN1:	acc,psw=acc
	if !i goto IN1
IN2:	acc,psw=acc
	if i goto IN2
	reg3=sio

	acc=reg3
	reg4,psw=sub(reg1)
	if z goto ECHO		; if CR goto new line

	acc=reg3
	reg4,psw=sub(reg0)
	acc=reg2
	if z goto acc,EXIT	; if ctrl-d goto exit
	sio=reg3		; else echo
	goto IN1


; exit
	org 0FF,07F
EXIT:	goto $
