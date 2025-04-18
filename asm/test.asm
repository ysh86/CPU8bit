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
	goto $
