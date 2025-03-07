//`timescale 1ns/1ps

module CPU8bit(
  input  logic       CLK,
  input  logic       nRESET,
  output logic [7:0] OUT
);

/*
  logic       CLK;
  logic       nRESET;
*/

  // delay
  logic nRESETbuf;
  always_ff @(posedge CLK) begin
    nRESETbuf <= nRESET;
  end

  logic [7:0] IN;
  assign IN = OUT;
  CPU CPU(.CLK, .nRESET(nRESETbuf), .PI(IN), .PO(OUT));

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
