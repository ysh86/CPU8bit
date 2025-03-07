typedef enum logic [3:0] {
  MOV,
  INC,
  INCC,
  DEC,
  DECC,
  ADD,
  ADC,
  SUB,
  SBC,
  AND,
  OR,
  XOR,
  RR,
  RL,
  RRC,
  RLC
} func_type;

module ALU(
  input logic [7:0] A, B,
  input logic       Cin,
  //input func_type   F,
  input logic [3:0] F,

  output logic [7:0] D,
  output logic       S, Z, H, PV, N, C
  );


  function zero(
    input logic [7:0] d
  );
    zero = ~|d;
  endfunction

  // 1: even
  // 0: odd
  function pty(
    input logic [7:0] d
  );
    pty = ~^d;
  endfunction

  function ov_one(
    input logic a,
    input logic d
  );
    ov_one = a ^ d;
  endfunction

  function ov_plus(
    input logic a,
    input logic b,
    input logic d
  );
    ov_plus = ~ov_one(a,b) & ov_one(a,d);
  endfunction

  function ov_minus(
    input logic a,
    input logic b,
    input logic d
  );
    ov_minus = ov_one(a,b) & ov_one(a,d);
  endfunction


  always_comb begin
    //unique case (func_type'(F))
    unique case (F)
      MOV: begin
        D = B;
        S = D[7];
        Z = zero(D);
        H = 1'b0;
        PV = pty(D);
        N = 1'b0;
        C = 1'b0;
      end

      INC: begin
        {C, D} = B + 8'b1;
        S = D[7];
        Z = zero(D);
        H = 1'b0; // not implemented
        PV = ov_one(B[7], D[7]);
        N = 1'b0;
      end
      INCC: begin
        {C, D} = B + Cin;
        S = D[7];
        Z = zero(D);
        H = 1'b0; // not implemented
        PV = ov_one(B[7], D[7]);
        N = 1'b0;
      end
      DEC: begin
        {C, D} = B - 8'b1;
        S = D[7];
        Z = zero(D);
        H = 1'b0; // not implemented
        PV = ov_one(B[7], D[7]);
        N = 1'b1;
      end
      DECC: begin
        {C, D} = B - Cin;
        S = D[7];
        Z = zero(D);
        H = 1'b0; // not implemented
        PV = ov_one(B[7], D[7]);
        N = 1'b1;
      end
      ADD: begin
        {C, D} = A + B;
        S = D[7];
        Z = zero(D);
        H = 1'b0; // not implemented
        PV = ov_plus(A[7], B[7], D[7]);
        N = 1'b0;
      end
      ADC: begin
        {C, D} = A + B + Cin;
        S = D[7];
        Z = zero(D);
        H = 1'b0; // not implemented
        PV = ov_plus(A[7], B[7], D[7]);
        N = 1'b0;
      end
      SUB: begin
        {C, D} = A - B;
        S = D[7];
        Z = zero(D);
        H = 1'b0; // not implemented
        PV = ov_minus(A[7], B[7], D[7]);
        N = 1'b1;
      end
      SBC: begin
        {C, D} = A - B - Cin;
        S = D[7];
        Z = zero(D);
        H = 1'b0; // not implemented
        PV = ov_minus(A[7], B[7], D[7]);
        N = 1'b1;
      end

      AND: begin
        D = A & B;
        S = D[7];
        Z = zero(D);
        H = 1'b0;
        PV = pty(D);
        N = 1'b0;
        C = 1'b0;
      end
      OR: begin
        D = A | B;
        S = D[7];
        Z = zero(D);
        H = 1'b0;
        PV = pty(D);
        N = 1'b0;
        C = 1'b0;
      end
      XOR: begin
        D = A ^ B;
        S = D[7];
        Z = zero(D);
        H = 1'b0;
        PV = pty(D);
        N = 1'b0;
        C = 1'b0;
      end

      RR: begin
        D = {Cin,B[7:1]};
        S = D[7];
        Z = zero(D);
        H = 1'b0;
        PV = pty(D);
        N = 1'b0;
        C = B[0];
      end
      RL: begin
        D = {B[6:0],Cin};
        S = D[7];
        Z = zero(D);
        H = 1'b0;
        PV = pty(D);
        N = 1'b0;
        C = B[7];
      end
      RRC: begin
        D = {B[0],B[7:1]};
        S = D[7];
        Z = zero(D);
        H = 1'b0;
        PV = pty(D);
        N = 1'b0;
        C = B[0];
      end
      RLC: begin
        D = {B[6:0],B[7]};
        S = D[7];
        Z = zero(D);
        H = 1'b0;
        PV = pty(D);
        N = 1'b0;
        C = B[7];
      end
    endcase
  end

endmodule
