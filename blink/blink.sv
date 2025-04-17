module blink(
  input  logic CLK,
  input  logic nRESET,
  output       LED_D2,
  output       LED_D1
);

// clock
logic sysclk;
logic locked;
assign LED_D2 = !locked;
PLL PLL(
  .inclk0(CLK),  // 33MHz
  .c0(sysclk),   // 33MHz
  .locked(locked)
);

// count 1sec
logic [24:0] counter;
always_ff @(posedge sysclk) begin
  if (!nRESET) begin
    counter <= 25'b0;
  end else begin
    counter <= counter + 25'b1;
  end
end

logic one_second;
assign one_second = counter[24];
assign LED_D1 = !one_second;

endmodule
