module JMP(
  input  logic [15:0] PRG,
  input  logic [7:0]  PSW,
  output logic        PAGE0,
  output logic        PAGE,
  output logic        JUMP
);

  logic       INS3;
  logic       IF;
  logic       NT;
  logic [2:0] CNDs;
  logic       AC;
  logic       PG;

  logic       cnd;
  logic       jmp;

  assign INS3 = PRG[15] & PRG[14];
  assign IF   = PRG[13];
  assign NT   = PRG[12];
  assign CNDs = PRG[11:9];
  assign AC   = PRG[8];
  assign PG   = PRG[7];

  /*
  always_comb begin
    unique case (CNDs)
      3'd0: cnd = PSW[0];
      3'd1: cnd = PSW[1];
      3'd2: cnd = PSW[2];
      3'd3: cnd = PSW[3];
      3'd4: cnd = PSW[4];
      3'd5: cnd = PSW[5];
      3'd6: cnd = PSW[6];
      3'd7: cnd = PSW[7];
    endcase
  end
  */
  assign cnd = PSW[CNDs];
  assign jmp = (~IF) | (IF & (NT ^ cnd));

  assign JUMP  = INS3 & jmp;
  assign PAGE  = JUMP & PG;
  assign PAGE0 = ~AC;

endmodule
