module memModule(input clk,
    input [7:0] io_M_Data,
    input [5:0] io_M_Addr,
    input  io_M_blockEnable,
    input  io_M_We,
    output[7:0] io_S_Data
);

  wire[7:0] T0;
  wire[7:0] T1;
  reg [7:0] syncMem [255:0];
  wire[7:0] T2;
  wire T3;
  wire T4;
  wire T5;
  wire[7:0] T7;
  wire[7:0] T8;
  wire T6;

`ifndef SYNTHESIS
// synthesis translate_off
  integer initvar;
  initial begin
    #0.002;
    for (initvar = 0; initvar < 256; initvar = initvar+1)
      syncMem[initvar] = {1{$random}};
  end
// synthesis translate_on
`endif

  assign io_S_Data = T0;
  assign T0 = T6 ? T1 : T1;
  assign T1 = syncMem[T8];
  assign T3 = T5 & T4;
  assign T4 = io_M_blockEnable == 1'h1;
  assign T5 = io_M_We == 1'h1;
  assign T7 = {2'h0, io_M_Addr};
  assign T8 = {2'h0, io_M_Addr};
  assign T6 = T3 ^ 1'h1;

  always @(posedge clk) begin
    if (T3)
      syncMem[T7] <= io_M_Data;
  end
endmodule

module memSPM(input clk,
    input [31:0] io_M_Data,
    input [7:0] io_M_Addr,
    input [3:0] io_M_ByteEn,
    input  io_M_We,
    output[31:0] io_S_Data
);

  wire T0;
  wire[5:0] T1;
  wire[7:0] T2;
  wire T3;
  wire[5:0] T4;
  wire[7:0] T5;
  wire T6;
  wire[5:0] T7;
  wire[7:0] T8;
  wire T9;
  wire[5:0] T10;
  wire[7:0] T11;
  wire[31:0] T12;
  wire[15:0] T13;
  wire[15:0] T14;
  wire[7:0] memModule_io_S_Data;
  wire[7:0] memModule_1_io_S_Data;
  wire[7:0] memModule_2_io_S_Data;
  wire[7:0] memModule_3_io_S_Data;


  assign T0 = io_M_ByteEn[2'h3:2'h3];
  assign T1 = io_M_Addr[3'h7:2'h2];
  assign T2 = io_M_Data[5'h1f:5'h18];
  assign T3 = io_M_ByteEn[2'h2:2'h2];
  assign T4 = io_M_Addr[3'h7:2'h2];
  assign T5 = io_M_Data[5'h17:5'h10];
  assign T6 = io_M_ByteEn[1'h1:1'h1];
  assign T7 = io_M_Addr[3'h7:2'h2];
  assign T8 = io_M_Data[4'hf:4'h8];
  assign T9 = io_M_ByteEn[1'h0:1'h0];
  assign T10 = io_M_Addr[3'h7:2'h2];
  assign T11 = io_M_Data[3'h7:1'h0];
  assign io_S_Data = T12;
  assign T12 = {T14, T13};
  assign T13 = {memModule_1_io_S_Data, memModule_io_S_Data};
  assign T14 = {memModule_3_io_S_Data, memModule_2_io_S_Data};
  memModule memModule(.clk(clk),
       .io_M_Data( T11 ),
       .io_M_Addr( T10 ),
       .io_M_blockEnable( T9 ),
       .io_M_We( io_M_We ),
       .io_S_Data( memModule_io_S_Data )
  );
  memModule memModule_1(.clk(clk),
       .io_M_Data( T8 ),
       .io_M_Addr( T7 ),
       .io_M_blockEnable( T6 ),
       .io_M_We( io_M_We ),
       .io_S_Data( memModule_1_io_S_Data )
  );
  memModule memModule_2(.clk(clk),
       .io_M_Data( T5 ),
       .io_M_Addr( T4 ),
       .io_M_blockEnable( T3 ),
       .io_M_We( io_M_We ),
       .io_S_Data( memModule_2_io_S_Data )
  );
  memModule memModule_3(.clk(clk),
       .io_M_Data( T2 ),
       .io_M_Addr( T1 ),
       .io_M_blockEnable( T0 ),
       .io_M_We( io_M_We ),
       .io_S_Data( memModule_3_io_S_Data )
  );
endmodule

