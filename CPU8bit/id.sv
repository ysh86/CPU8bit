module ID(
  input logic [15:0] PRG,
  output logic [1:0] INS,
  output logic [4:0] G1,
  output logic [3:0] FNC,
  output logic [4:0] G2,
  output logic       G3,
  output logic [7:0] CNS,
  output logic       aH,
  output logic       aL,
  output logic       r,
  output logic       w
);

  assign INS = PRG[15:14];
  assign G1  = PRG[13:9];
  assign FNC = PRG[8:5];
  assign G2  = PRG[4:0];
  assign G3  = PRG[14];
  assign CNS = PRG[7:0];

  assign aH = (INS    != 2'b11) & (G1 == 5'd2);
  assign aL = (INS    != 2'b11) & (G1 == 5'd3);
  assign r  = (INS[1] == 1'b0)  & (G2 == 5'd4);
  assign w  = (INS    != 2'b11) & (G1 == 5'd4);

endmodule
