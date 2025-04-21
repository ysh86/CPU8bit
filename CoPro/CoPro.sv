module CoPro(
  input  logic       CLK,
  input  logic       nRESET,
  input  logic [7:0] PI,
  output logic [7:0] PO,
  output logic       LED_D2
);

// clock
logic sysclk;
logic locked;
assign LED_D2 = !locked;
PLL clock(
  .inclk0(CLK),  // 33MHz
  .c0(sysclk),   // 33MHz
  .locked(locked)
);

// reset
logic nRESETbuf;
always_ff @(posedge sysclk) begin
  nRESETbuf <= nRESET;
end

// UART
logic [7:0] data_tx;
logic [7:0] data_rx;
logic wr, rd;
logic txmt, txfl, rxmt, rxfl;
jtag_uart uart(
  .clk_i(sysclk),
  .nreset_i(nRESETbuf),
  .nwr_i(wr),
  .data_i(data_tx),
  .rd_i(rd),
  .data_o(data_rx),
  .txmt(txmt),
  .txfl(txfl),
  .rxmt(rxmt),
  .rxfl(rxfl)
);

// I/O
//assign data_tx = PI;
assign PO = data_rx;

// Serial flash
logic [23:0] addr;
logic        read;
logic        rden;
logic        busy;
logic        data_valid;
logic [7:0]  dataout;
EPCS4 flash(
  .addr(addr),
  .clkin(sysclk),
  .rden(rden),
  .read(read),
  .busy(busy),
  .data_valid(data_valid),
  .dataout(dataout)
);

// FSM demo

// states
parameter S_INIT = 0, S_FETCH = 7, S_FETCHED = 8, S_STROBE = 1, S_CNT = 2, S_WAIT = 3, S_READ = 4, S_SETTLE = 5, S_WRITE = 6;
logic [3:0] state;

always_ff @(posedge sysclk) begin
  if (!nRESETbuf) begin
    state <= S_INIT;
	 addr  <= 24'h20000;
	 read  <= 1'b0;
	 rden  <= 1'b0;
    wr    <= 1'b1;
    rd    <= 1'b0;
  end else begin
    unique case(state)
      S_INIT: begin
        addr    <= 24'h20000;
        read    <= 1'b1;
        rden    <= 1'b1;
        wr      <= 1'b1;
        rd      <= 1'b0;
        state   <= S_FETCH;
      end
      S_FETCH: begin
        read    <= 1'b0;
        rden    <= 1'b0;
        state   <= S_FETCHED;
      end
		S_FETCHED: begin
		  if (data_valid) begin
		    data_tx <= dataout;
		  end
		  if (!busy) begin
		    state <= S_STROBE;
		  end
		end
      S_STROBE: begin
        if (data_tx != 8'hff) begin
          wr    <= 1'b0;
		  end
        state   <= S_CNT;
      end
      S_CNT: begin
        if (wr == 1'b0) begin
		    // print mem
          addr  <= addr + 1'b1;
		    read  <= 1'b1;
			 rden  <= 1'b1;
          wr    <= 1'b1;
          state <= S_FETCH;
        end else
          state <= S_WAIT;
      end
      S_WAIT: begin // input
        if (!rxmt) begin
          rd    <= 1'b1;
          state <= S_READ;
        end else begin
          wr    <= 1'b1;
          rd    <= 1'b0;
          state <= S_WAIT;
        end
      end
      S_READ: begin
        rd      <= 1'b0;
        data_tx <= data_rx; // echo back
        state   <= S_SETTLE;
      end
      S_SETTLE: begin
        wr      <= 1'b0;
        state   <= S_WRITE;
      end
      S_WRITE: begin
        wr      <= 1'b1;
        state   <= S_WAIT;
      end
      default: begin
        state   <= S_INIT;
      end
    endcase
  end
end

endmodule
