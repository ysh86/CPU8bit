module PCT(
  input  logic        CLK,
  input  logic        nRESET,
  input  logic [7:0]  PSW,
  input  logic [7:0]  PCH,
  output logic [15:0] PRG
);

  logic [14:0] PC;
  rom ROM(.address(PC[7:0]), .q(PRG));

  logic PAGE0;
  logic PAGE;
  logic JUMP;
  JMP JMP(.PRG, .PSW, .PAGE0, .PAGE, .JUMP); 
  
  logic [7:0]  PC_page;
  logic [14:0] PC_inc;
  assign PC_page = (PAGE) ? ((PAGE0) ? 8'b0 : PCH) : PC[14:7];
  assign PC_inc  = PC + 15'd1;

  logic [14:0] PC_next;
  assign PC_next = (JUMP) ? {PC_page,PRG[6:0]} : PC_inc;

  always_ff @(posedge CLK) begin
    if (~nRESET) begin
      PC <= 15'd0;
    end else begin
      PC <= PC_next;
    end
  end

endmodule
