`define DIV_BITS 15 // 33MHz div 32768 -> 1007Hz

module SW (
  input  logic CLK_33,
  input  logic nIN,
  output logic nOUT
);

logic [`DIV_BITS-1:0] div_counter;
logic                 div_clk;
assign div_clk = div_counter[`DIV_BITS-1];

always_ff @(posedge CLK_33) begin
  div_counter <= div_counter + `DIV_BITS'b1;
end

always_ff @(posedge div_clk) begin
  nOUT <= nIN;
end

endmodule
