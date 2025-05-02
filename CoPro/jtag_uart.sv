/*
#   jtag_uart.v - This module provides an 8-bit parallel FIFO interface to
#                 the Altera Virtual JTAG module.
#
#   Description: Follows the design pattern recommended in Altera's Virtual
#    JTAG Megafunction user guide.  Links the VJTAG to two 64-byte FIFO's
#  
#   Copyright (C) 2014  Binary Logic (nhi.phan.logic at gmail.com).
#
#   This file is part of the Virtual JTAG UART toolkit
#   
#   Virtual UART is free software: you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation, either version 3 of the License, or
#   (at your option) any later version.
#   
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#   
#   You should have received a copy of the GNU General Public License
#   along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
*/

module jtag_uart(
  input  logic clk_i,
  input  logic nreset_i,
  input  logic nwr_i,
  input  logic [7:0] data_i,
  input  logic rd_i,
  output logic [7:0] data_o,
  output logic txmt,
  output logic txfl,
  output logic rxmt,
  output logic rxfl
);

//=======================================================
// JTAG Command Table
//=======================================================
`define TX     2'b00
`define RX     2'b01
`define STATUS 2'b10
`define BYPASS 2'b11

//=======================================================
//  REG/WIRE declarations
//=======================================================
// Virtual JTAG signals, see Altera's Virtual JTAG Megafunction user guide for infoo
logic [1:0] ir_out, ir_in;

logic tdo, tck, tdi;
logic virtual_state_cdr;
logic virtual_state_sdr;
logic virtual_state_udr;
logic virtual_state_uir;

// VJTAG registers
logic [7:0] out_data;         // Output buffer signals
logic [7:0] shift_buffer = 0; // Internal buffer for shifting in/out
logic cdr_delayed;            // Capture data register delayed by half a clock cycle
logic sdr_delayed;            // shift data register delayed by half a clock cycle
logic [1:0] bypass;           // Bypass register
logic TXmt, TXfl;             // Transmit empty, full signals
logic RXmt, RXfl;             // Receive empty, full signals

// Stored instruction register, default to bypass
logic [1:0] ir = `BYPASS;

// Buffer management signals
logic out_full, out_empty;
logic in_full, in_empty;

//=======================================================
//  Outputs
//=======================================================

assign ir_out = ir_in;  // Just pass the IR out
// If the capture instruction register points to the bypass register then output
// the bypass register, or the shift register
assign tdo = (ir == `BYPASS) ? bypass[0] : shift_buffer[0];
assign txmt = TXmt;
assign txfl = TXfl;
assign rxmt = RXmt;
assign rxfl = RXfl;

//=======================================================
//  Structural coding
//=======================================================

// Virtual JTAG - prep signals are on rising edge TCK, output on falling edge
// This connects to the internal signal trap hub - see the VJTAG MF User Guide
VJTAG vjtag(
  .ir_out(ir_out),
  .tdo(tdo),
  .ir_in(ir_in),
  .tck(tck),
  .tdi(tdi),
  .virtual_state_cdr(virtual_state_cdr),
  .virtual_state_cir(),
  .virtual_state_e1dr(),
  .virtual_state_e2dr(),
  .virtual_state_pdr(),
  .virtual_state_sdr(virtual_state_sdr),
  .virtual_state_udr(virtual_state_udr),
  .virtual_state_uir(virtual_state_uir)
);

// Output buffer FIFO, write clock is system clock
// read clock is the VJTAG TCK clock
FIFO out(
  .aclr(!nreset_i),
  // Write signals
  .data(data_i),
  .wrclk(clk_i),
  .wrreq(!nwr_i),
  .wrfull(out_full),
  // Read signals
  .rdclk(!tck),
  .rdreq(virtual_state_cdr & (ir == `TX)),
  .q(out_data),
  .rdempty(out_empty)
);

// Input buffer FIFO, write clock is VJTAG TCK clock
// read clock is the system clock
FIFO in(
  .aclr(!nreset_i),
  // Write signals
  .data(shift_buffer),
  .wrclk(!tck),
  .wrreq(virtual_state_udr & (ir == `RX)),
  .wrfull(in_full),
  // Read signals
  .rdclk(!clk_i),
  .rdreq(rd_i),
  .q(data_o),
  .rdempty(in_empty)
);

//=======================================================
//  Procedural coding
//=======================================================

// Set the full/empty signals
always_ff @(posedge tck) begin
  TXmt <= out_empty;
  RXfl <= in_full;
end

// Set the full/empty signals
always_ff @(posedge clk_i) begin
  TXfl <= out_full;
  RXmt <= in_empty;
end

// VJTAG Controls for UART output
always_ff @(negedge tck) begin
  // Delay the CDR signal by one half clock cycle 
  cdr_delayed <= virtual_state_cdr;
  sdr_delayed <= virtual_state_sdr;
end

// Capture the instruction provided
always_ff @(negedge tck) begin
  if (virtual_state_uir) ir <= ir_in;
end

// Data is clocked out on the falling edge, rising edge is for prep
always @(posedge tck) begin
  unique case (ir)
    // Process output
    `TX: begin
      if (cdr_delayed)
        shift_buffer <= out_data;
      else if (sdr_delayed)
        shift_buffer <= {tdi,shift_buffer[7:1]};
    end
    // Process input
    `RX: begin
      if (sdr_delayed)
        shift_buffer <= {tdi,shift_buffer[7:1]};
    end
    // Process status request (only 4 bits are required to be shifted)
    `STATUS: begin
      if (cdr_delayed)
        shift_buffer <= {4'b0000, RXfl, RXmt, TXfl, TXmt};
      else if (sdr_delayed)
        shift_buffer <= {tdi,shift_buffer[7:1]};
    end
    // Process input
    `BYPASS: begin
      if (sdr_delayed)
        bypass <= {tdi,bypass[1:1]};
    end
  endcase
end

endmodule
