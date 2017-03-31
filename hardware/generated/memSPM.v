module memSPM(input clk, input reset,
    input [31:0] io_M_Data,
    input [15:0] io_M_Addr,
    input [3:0] io_M_ByteEn,
    input  io_M_We,
    output[31:0] io_S_Data
);

  wire[31:0] T0;
  reg [31:0] dataReg;
  wire[31:0] T1;
  reg [31:0] syncMem [127:0];
  wire[31:0] T2;
  wire T3;
  wire[6:0] T5;
  wire[6:0] T6;
  wire T4;

`ifndef SYNTHESIS
// synthesis translate_off
  integer initvar;
  initial begin
    #0.002;
    dataReg = {1{$random}};
    for (initvar = 0; initvar < 128; initvar = initvar+1)
      syncMem[initvar] = {1{$random}};
  end
// synthesis translate_on
`endif

  assign io_S_Data = T0;
  assign T0 = T4 ? T1 : dataReg;
  assign T1 = syncMem[T6];
  assign T3 = io_M_We == 1'h1;
  assign T5 = io_M_Addr[3'h6:1'h0];
  assign T6 = io_M_Addr[3'h6:1'h0];
  assign T4 = T3 ^ 1'h1;

  always @(posedge clk) begin
    dataReg <= 32'h0;
    if (T3)
      syncMem[T5] <= io_M_Data;
  end
endmodule

