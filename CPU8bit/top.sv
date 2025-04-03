//`timescale 1ns/1ps

module CPU8bit(
  input  logic       CLK,
  input  logic       SW4,
  output logic [7:0] LED1_D,
  output logic [7:0] LED2_D
);

/*
  logic       CLK;
  logic       nRESET;
*/

  // 33.000 MHz -> 16.5 MHz
  logic CLK_16;
  always_ff @(posedge CLK) begin
    CLK_16 <= ~CLK_16;
  end

  logic nRESET;
  SW RSTSW(.nIN(SW4), .nOUT(nRESET));

  // delay
  logic nRESETbuf;
  always_ff @(posedge CLK_16) begin
    nRESETbuf <= nRESET;
  end

  logic [7:0] OUT;
  LED LED2(.IN(OUT[7:4]), .OUT(LED2_D));
  LED LED1(.IN(OUT[3:0]), .OUT(LED1_D));

  logic [7:0] IN;
  assign IN = OUT;
  CPU CPU(.CLK(CLK_16), .nRESET(nRESETbuf), .PI(IN), .PO(OUT));

/*
  always #10 CLK = ~CLK;

  initial begin
    nRESET = 0;
    CLK = 0;
	#20
	nRESET = 1;
	#400
	$finish;
  end

  initial begin
    $monitor("OUT=%02x", OUT);
	$dumpfile("OUT.vcd");
	$dumpvars(0, CPU8bit);
  end
*/

endmodule
