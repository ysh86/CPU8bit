module MB(
  input  logic       CLK,
  input  logic       SW4,
  input  logic [7:0] ExtD,
  output logic       ExtD1,
  output logic       ExtD0,
  output logic [7:0] LED1_D,
  output logic [7:0] LED2_D,
  output logic       LED3_D
);

// clock
logic CLK_33;
assign CLK_33 = CLK;
assign ExtD1 = CLK_33;

// reset
SW RSTSW(.CLK_33(CLK_33), .nIN(SW4), .nOUT(ExtD0));

// LED
assign LED3_D = !ExtD0;
LED LED2(.IN(ExtD[7:4]), .OUT(LED2_D));
LED LED1(.IN(ExtD[3:0]), .OUT(LED1_D));

endmodule
