module PCT(
  input  logic [14:0] PC,
  input  logic        PAGE0,
  input  logic        PAGE,
  input  logic        JUMP,
  input  logic [7:0]  ACC,
  input  logic [6:0]  addr,
  output logic [14:0] PC_next
);

  logic [7:0]  PC_page;
  logic [14:0] PC_inc;
  assign PC_page = (PAGE) ? ((PAGE0) ? 8'b0 : ACC) : PC[14:7];
  assign PC_inc  = PC + 15'd1;

  assign PC_next = (JUMP) ? {PC_page,addr} : PC_inc;

endmodule
