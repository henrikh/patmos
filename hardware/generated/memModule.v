module memModule(input clk, input reset,
    input [7:0] io_M_Data,
    input [7:0] io_M_Addr,
    input  io_M_blockEnable,
    input  io_M_We,
    output[7:0] io_S_Data
);

  wire[7:0] T0;
  reg [7:0] dataReg;
  wire[7:0] T1;
  reg [7:0] syncMem [255:0];
  wire[7:0] T2;
  wire T3;
  wire T4;
  wire T5;
  wire T6;

`ifndef SYNTHESIS
// synthesis translate_off
  integer initvar;
  initial begin
    #0.002;
    dataReg = {1{$random}};
    for (initvar = 0; initvar < 256; initvar = initvar+1)
      syncMem[initvar] = {1{$random}};
  end
// synthesis translate_on
`endif

  assign io_S_Data = T0;
  assign T0 = T6 ? T1 : dataReg;
  assign T1 = syncMem[io_M_Addr];
  assign T3 = T5 & T4;
  assign T4 = io_M_blockEnable == 1'h1;
  assign T5 = io_M_We == 1'h1;
  assign T6 = T3 ^ 1'h1;

  always @(posedge clk) begin
    dataReg <= 8'h0;
    if (T3)
      syncMem[io_M_Addr] <= io_M_Data;
  end
endmodule

