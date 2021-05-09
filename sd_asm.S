	.code16

	.text

.macro toggle_clk
    or      $0x2, %al
    out     %ax,%dx     // write P1LTCH clk high
    and     $0xfd, %al
    out     %ax,%dx     // write P1LTCH clk low
.endm

.macro test_bit_and_shift bit
    test    $\bit,%cl   // is bit set?
    jz     3f           // nope? jump to b80_ns

    or      $0x1,%al    // yes, so turn on bit 0 of P1LTCH
    jmp     2f    // toggle clk

3:
    and     $0xfe,%al   // turn off bit 0 of P1LTCH
2:
    out     %ax,%dx     // write P1LTCH clk low
    toggle_clk
.endm

.macro sample_input_bit_and_shift_data
    sal     $1,%bl     // bl << 1

    mov     $0xff5a,%dx // fetch current DIN
    in      %dx,%ax     //
    test    $0x40,%al   // if(DIN)
    jz     2f      //
    or     $1, %bl         // bl++
2:
    mov     $0xff56,%dx // fetch current P1LTCH value
    in      %dx,%ax     //
    or      $0x1,%al    // DI high
    toggle_clk
.endm


.global xmit_mmc
/*-----------------------------------------------------------------------*/
/* Transmit a BYTE to the MMC (bitbanging)                               */
/*-----------------------------------------------------------------------*/
//void xmit_mmc (
//	uint8_t d			/* Data to be sent */
//);
xmit_mmc:
	mov     %sp,%bx
	mov     2(%bx),%cx

    mov     $0xff56,%dx

    in      %dx,%ax     // fetch current P1LTCH value

    test_bit_and_shift 0x80
    test_bit_and_shift 0x40
    test_bit_and_shift 0x20
    test_bit_and_shift 0x10
    test_bit_and_shift 0x08
    test_bit_and_shift 0x04
    test_bit_and_shift 0x02
    test_bit_and_shift 0x01

	ret

.global rcvr_mmc
/*-----------------------------------------------------------------------*/
/* Receive a BYTE from the MMC (bitbanging)                              */
/*-----------------------------------------------------------------------*/
//uint8_t rcvr_mmc (void)
rcvr_mmc:
    xor     %bx,%bx

    sample_input_bit_and_shift_data
    sample_input_bit_and_shift_data
    sample_input_bit_and_shift_data
    sample_input_bit_and_shift_data
    sample_input_bit_and_shift_data
    sample_input_bit_and_shift_data
    sample_input_bit_and_shift_data
    sample_input_bit_and_shift_data

    mov     %bx, %ax
    ret

.global skip_mmc
/*-----------------------------------------------------------------------*/
/* Skip BYTEs on the MMC (bitbanging)                                    */
/*-----------------------------------------------------------------------*/
//void skip_mmc (
//	uint16_t n		/* Number of BYTEs to skip */
//);
skip_mmc:
    mov     %sp,%bx
	mov     2(%bx),%cx

    mov     $0xff56,%dx // fetch current P1LTCH value
    in      %dx,%ax     //
    or      $0x1,%al    // DI high
    out     %ax,%dx     // write P1LTCH DI high

loop:
    toggle_clk
    toggle_clk
    toggle_clk
    toggle_clk
    toggle_clk
    toggle_clk
    toggle_clk
    toggle_clk

    dec     %cx
    jnz loop
    ret