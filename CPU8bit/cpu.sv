module CPU(
  input  logic       CLK,
  input  logic       nRESET,
  input  logic [7:0] PI,
  output logic [7:0] PO
);

  // REG
  logic [7:0] ACC;
  logic [7:0] PSW;
  logic [7:0] ADH;
  logic [7:0] ADL;
  logic [7:0] MEM;
  logic [7:0] TMR;
  logic [7:0] PIO;
  logic [7:0] SIO;
  logic [7:0] REG[0:7];

  // RAM
  logic [9:0] address;
  logic [7:0] data;
  logic       we;
  logic [7:0] q;
  ram RAM(.address, .data, .we, .q);

  // fetch: ROM
  logic [14:0] PC; // word addr.
  logic [15:0] PRG;
  rom ROM(.address(PC[7:0]), .q(PRG));

  // decode
  logic [1:0] INS;
  logic [4:0] G1;
  logic [3:0] FNC;
  logic [4:0] G2;
  logic       G3;
  logic [7:0] CNS;
  logic       w;
  ID ID(.PRG, .INS, .G1, .FNC, .G2, .G3, .CNS, .w);
  logic PAGE0;
  logic PAGE;
  logic JUMP;
  JMP JMP(.PRG, .PSW, .PAGE0, .PAGE, .JUMP);

  // exec
  logic [7:0] B2;
  always_comb begin
    unique case (G2)
      5'd0:    B2 = ACC;
      5'd1:    B2 = PSW;
      5'd2:    B2 = ADH;
      5'd3:    B2 = ADL;
      5'd4:    B2 = q;
      5'd5:    B2 = TMR;
      5'd6:    B2 = PI;
      5'd7:    B2 = SIO;
      5'd16:   B2 = REG[0];
      5'd17:   B2 = REG[1];
      5'd18:   B2 = REG[2];
      5'd19:   B2 = REG[3];
      5'd20:   B2 = REG[4];
      5'd21:   B2 = REG[5];
      5'd22:   B2 = REG[6];
      5'd23:   B2 = REG[7];
      default: B2 = 8'hff; // invalid
    endcase
  end

  logic [7:0] D;
  logic [7:0] S;
  ALU ALU(
    .A(ACC),
    .B(B2),
    .Cin(PSW[0]),
    .F(FNC),

    .D,
    .S(S[7]),
    .Z(S[6]),
    .H(S[4]),
    .PV(S[2]),
    .N(S[1]),
    .C(S[0])
  );
  assign S[5] = 1'b0; // not implemented
  assign S[3] = 1'b0; // not implemented

  // write back: src
  logic [7:0] B1;
  assign B1 = (INS != 2'b10) ? D : CNS;

  // write back: dst REG
  always_ff @(posedge CLK) begin
    if (~nRESET) begin
      ACC <= 8'b0000_0000;
      PSW <= 8'b0000_0000;
      ADH <= 8'b0000_0000;
      ADL <= 8'b0000_0000;
      MEM <= 8'b0000_0000;
      TMR <= 8'b0000_0000;
      PIO <= 8'b0000_0000;
      SIO <= 8'b0000_0000;
    end else if (INS != 2'b11) begin
      if (G3)    PSW    <= S;
      unique case (G1)
        5'd0:    ACC    <= B1;
        5'd1:    if (~G3) PSW <= B1;
        5'd2:    ADH    <= B1;
        5'd3:    ADL    <= B1;
        5'd4:    MEM    <= B1; // and write B1 to RAM
        5'd5:    TMR    <= B1;
        5'd6:    PIO    <= B1;
        5'd7:    SIO    <= B1;
        5'd16:   REG[0] <= B1;
        5'd17:   REG[1] <= B1;
        5'd18:   REG[2] <= B1;
        5'd19:   REG[3] <= B1;
        5'd20:   REG[4] <= B1;
        5'd21:   REG[5] <= B1;
        5'd22:   REG[6] <= B1;
        5'd23:   REG[7] <= B1;
        default:             ; // do nothing
      endcase
    end
  end

  // write back: dst RAM
  assign address = {ADH[1:0],ADL[7:0]};
  assign data = MEM;
  logic w_buf;
  always_ff @(posedge CLK) begin
    if (~nRESET) w_buf <= 1'b0;
    else         w_buf <= w;
  end
  assign we = (nRESET & CLK) & w_buf;

  // write back: dst PO
  assign PO = PIO;

  // PCT: update PC
  logic [14:0] PC_next;
  PCT PCT(.PC, .PAGE0, .PAGE, .JUMP, .ACC, .addr(PRG[6:0]), .PC_next);
  always_ff @(posedge CLK) begin
    if (~nRESET) PC <= 15'd0;
    else         PC <= PC_next;
  end

endmodule
