#include "memSPM.h"

void memSPM_t::init ( val_t rand_init ) {
  this->__srand(rand_init);
  memSPM_memModule_3__syncMem.randomize(&__rand_seed);
  memSPM_memModule_2__syncMem.randomize(&__rand_seed);
  memSPM_memModule_1__syncMem.randomize(&__rand_seed);
  memSPM_memModule__syncMem.randomize(&__rand_seed);
  clk.len = 1;
  clk.cnt = clk.len;
  clk.values[0] = 0;
}


int memSPM_t::clock ( dat_t<1> reset ) {
  uint32_t min = ((uint32_t)1<<31)-1;
  if (clk.cnt < min) min = clk.cnt;
  clk.cnt-=min;
  if (clk.cnt == 0) clock_lo( reset );
  if (clk.cnt == 0) clock_hi( reset );
  if (clk.cnt == 0) clk.cnt = clk.len;
  return min;
}


void memSPM_t::print ( FILE* f ) {
}
void memSPM_t::print ( std::ostream& s ) {
}


void memSPM_t::dump_init ( FILE* f ) {
}


void memSPM_t::dump ( FILE* f, int t ) {
}




void memSPM_t::clock_lo ( dat_t<1> reset, bool assert_fire ) {
  val_t T0;
  { T0 = memSPM__io_M_Data.values[0] >> 24;}
  T0 = T0 & 0xffL;
  { memSPM_memModule_3__io_M_Data.values[0] = T0;}
  val_t T1;
  T1 = (memSPM__io_M_ByteEn.values[0] >> 3) & 1;
  { memSPM_memModule_3__io_M_blockEnable.values[0] = T1;}
  val_t T2;
  T2 = memSPM_memModule_3__io_M_blockEnable.values[0] == 0x1L;
  { memSPM_memModule_3__io_M_We.values[0] = memSPM__io_M_We.values[0];}
  val_t T3;
  T3 = memSPM_memModule_3__io_M_We.values[0] == 0x1L;
  { T4.values[0] = T3 & T2;}
  val_t T5;
  { T5 = memSPM__io_M_Addr.values[0] >> 2;}
  T5 = T5 & 0x3fL;
  { memSPM_memModule_3__io_M_Addr.values[0] = T5;}
  { T6.values[0] = memSPM_memModule_3__io_M_Addr.values[0] | 0x0L << 6;}
  val_t T7;
  { T7 = memSPM__io_M_Data.values[0] >> 16;}
  T7 = T7 & 0xffL;
  { memSPM_memModule_2__io_M_Data.values[0] = T7;}
  val_t T8;
  T8 = (memSPM__io_M_ByteEn.values[0] >> 2) & 1;
  { memSPM_memModule_2__io_M_blockEnable.values[0] = T8;}
  val_t T9;
  T9 = memSPM_memModule_2__io_M_blockEnable.values[0] == 0x1L;
  { memSPM_memModule_2__io_M_We.values[0] = memSPM__io_M_We.values[0];}
  val_t T10;
  T10 = memSPM_memModule_2__io_M_We.values[0] == 0x1L;
  { T11.values[0] = T10 & T9;}
  val_t T12;
  { T12 = memSPM__io_M_Addr.values[0] >> 2;}
  T12 = T12 & 0x3fL;
  { memSPM_memModule_2__io_M_Addr.values[0] = T12;}
  { T13.values[0] = memSPM_memModule_2__io_M_Addr.values[0] | 0x0L << 6;}
  val_t T14;
  { T14 = memSPM__io_M_Data.values[0] >> 8;}
  T14 = T14 & 0xffL;
  { memSPM_memModule_1__io_M_Data.values[0] = T14;}
  val_t T15;
  T15 = (memSPM__io_M_ByteEn.values[0] >> 1) & 1;
  { memSPM_memModule_1__io_M_blockEnable.values[0] = T15;}
  val_t T16;
  T16 = memSPM_memModule_1__io_M_blockEnable.values[0] == 0x1L;
  { memSPM_memModule_1__io_M_We.values[0] = memSPM__io_M_We.values[0];}
  val_t T17;
  T17 = memSPM_memModule_1__io_M_We.values[0] == 0x1L;
  { T18.values[0] = T17 & T16;}
  val_t T19;
  { T19 = memSPM__io_M_Addr.values[0] >> 2;}
  T19 = T19 & 0x3fL;
  { memSPM_memModule_1__io_M_Addr.values[0] = T19;}
  { T20.values[0] = memSPM_memModule_1__io_M_Addr.values[0] | 0x0L << 6;}
  val_t T21;
  { T21 = memSPM__io_M_Data.values[0];}
  T21 = T21 & 0xffL;
  { memSPM_memModule__io_M_Data.values[0] = T21;}
  val_t T22;
  T22 = (memSPM__io_M_ByteEn.values[0] >> 0) & 1;
  { memSPM_memModule__io_M_blockEnable.values[0] = T22;}
  val_t T23;
  T23 = memSPM_memModule__io_M_blockEnable.values[0] == 0x1L;
  { memSPM_memModule__io_M_We.values[0] = memSPM__io_M_We.values[0];}
  val_t T24;
  T24 = memSPM_memModule__io_M_We.values[0] == 0x1L;
  { T25.values[0] = T24 & T23;}
  val_t T26;
  { T26 = memSPM__io_M_Addr.values[0] >> 2;}
  T26 = T26 & 0x3fL;
  { memSPM_memModule__io_M_Addr.values[0] = T26;}
  { T27.values[0] = memSPM_memModule__io_M_Addr.values[0] | 0x0L << 6;}
  val_t T28;
  { T28 = memSPM_memModule__io_M_Addr.values[0] | 0x0L << 6;}
  val_t T29;
  { T29 = memSPM_memModule__syncMem.get(T28, 0);}
  val_t T30;
  { T30 = T25.values[0] ^ 0x1L;}
  val_t T31;
  { T31 = TERNARY_1(T30, T29, T29);}
  { memSPM_memModule__io_S_Data.values[0] = T31;}
  val_t T32;
  { T32 = memSPM_memModule_1__io_M_Addr.values[0] | 0x0L << 6;}
  val_t T33;
  { T33 = memSPM_memModule_1__syncMem.get(T32, 0);}
  val_t T34;
  { T34 = T18.values[0] ^ 0x1L;}
  val_t T35;
  { T35 = TERNARY_1(T34, T33, T33);}
  { memSPM_memModule_1__io_S_Data.values[0] = T35;}
  val_t T36;
  { T36 = memSPM_memModule__io_S_Data.values[0] | memSPM_memModule_1__io_S_Data.values[0] << 8;}
  val_t T37;
  { T37 = memSPM_memModule_2__io_M_Addr.values[0] | 0x0L << 6;}
  val_t T38;
  { T38 = memSPM_memModule_2__syncMem.get(T37, 0);}
  val_t T39;
  { T39 = T11.values[0] ^ 0x1L;}
  val_t T40;
  { T40 = TERNARY_1(T39, T38, T38);}
  { memSPM_memModule_2__io_S_Data.values[0] = T40;}
  val_t T41;
  { T41 = memSPM_memModule_3__io_M_Addr.values[0] | 0x0L << 6;}
  val_t T42;
  { T42 = memSPM_memModule_3__syncMem.get(T41, 0);}
  val_t T43;
  { T43 = T4.values[0] ^ 0x1L;}
  val_t T44;
  { T44 = TERNARY_1(T43, T42, T42);}
  { memSPM_memModule_3__io_S_Data.values[0] = T44;}
  val_t T45;
  { T45 = memSPM_memModule_2__io_S_Data.values[0] | memSPM_memModule_3__io_S_Data.values[0] << 8;}
  val_t T46;
  { T46 = T36 | T45 << 16;}
  { memSPM__io_S_Data.values[0] = T46;}
}


void memSPM_t::clock_hi ( dat_t<1> reset ) {
  { if (T4.values[0]) memSPM_memModule_3__syncMem.put(T6.values[0], 0, memSPM_memModule_3__io_M_Data.values[0]);}
  { if (T11.values[0]) memSPM_memModule_2__syncMem.put(T13.values[0], 0, memSPM_memModule_2__io_M_Data.values[0]);}
  { if (T18.values[0]) memSPM_memModule_1__syncMem.put(T20.values[0], 0, memSPM_memModule_1__io_M_Data.values[0]);}
  { if (T25.values[0]) memSPM_memModule__syncMem.put(T27.values[0], 0, memSPM_memModule__io_M_Data.values[0]);}
}


void memSPM_api_t::init_sim_data (  ) {
  sim_data.inputs.clear();
  sim_data.outputs.clear();
  sim_data.signals.clear();
  memSPM_t* mod = dynamic_cast<memSPM_t*>(module);
  assert(mod);
  sim_data.inputs.push_back(new dat_api<32>(&mod->memSPM__io_M_Data));
  sim_data.inputs.push_back(new dat_api<8>(&mod->memSPM__io_M_Addr));
  sim_data.inputs.push_back(new dat_api<4>(&mod->memSPM__io_M_ByteEn));
  sim_data.inputs.push_back(new dat_api<1>(&mod->memSPM__io_M_We));
  sim_data.outputs.push_back(new dat_api<32>(&mod->memSPM__io_S_Data));
  sim_data.signals.push_back(new dat_api<8>(&mod->memSPM_memModule_3__io_M_Data));
  sim_data.signal_map["memSPM.memModule_3.io_M_Data"] = 0;
  sim_data.signals.push_back(new dat_api<1>(&mod->memSPM_memModule_3__io_M_blockEnable));
  sim_data.signal_map["memSPM.memModule_3.io_M_blockEnable"] = 1;
  sim_data.signals.push_back(new dat_api<1>(&mod->memSPM_memModule_3__io_M_We));
  sim_data.signal_map["memSPM.memModule_3.io_M_We"] = 2;
  sim_data.signals.push_back(new dat_api<6>(&mod->memSPM_memModule_3__io_M_Addr));
  sim_data.signal_map["memSPM.memModule_3.io_M_Addr"] = 3;
  std::string memSPM_memModule_3__syncMem_path = "memSPM.memModule_3.syncMem";
  for (size_t i = 0 ; i < 256 ; i++) {
    sim_data.signals.push_back(new dat_api<8>(&mod->memSPM_memModule_3__syncMem.contents[i]));
    sim_data.signal_map[memSPM_memModule_3__syncMem_path+"["+itos(i,false)+"]"] = 4+i;
  }
  sim_data.signals.push_back(new dat_api<8>(&mod->memSPM_memModule_2__io_M_Data));
  sim_data.signal_map["memSPM.memModule_2.io_M_Data"] = 260;
  sim_data.signals.push_back(new dat_api<1>(&mod->memSPM_memModule_2__io_M_blockEnable));
  sim_data.signal_map["memSPM.memModule_2.io_M_blockEnable"] = 261;
  sim_data.signals.push_back(new dat_api<1>(&mod->memSPM_memModule_2__io_M_We));
  sim_data.signal_map["memSPM.memModule_2.io_M_We"] = 262;
  sim_data.signals.push_back(new dat_api<6>(&mod->memSPM_memModule_2__io_M_Addr));
  sim_data.signal_map["memSPM.memModule_2.io_M_Addr"] = 263;
  std::string memSPM_memModule_2__syncMem_path = "memSPM.memModule_2.syncMem";
  for (size_t i = 0 ; i < 256 ; i++) {
    sim_data.signals.push_back(new dat_api<8>(&mod->memSPM_memModule_2__syncMem.contents[i]));
    sim_data.signal_map[memSPM_memModule_2__syncMem_path+"["+itos(i,false)+"]"] = 264+i;
  }
  sim_data.signals.push_back(new dat_api<8>(&mod->memSPM_memModule_1__io_M_Data));
  sim_data.signal_map["memSPM.memModule_1.io_M_Data"] = 520;
  sim_data.signals.push_back(new dat_api<1>(&mod->memSPM_memModule_1__io_M_blockEnable));
  sim_data.signal_map["memSPM.memModule_1.io_M_blockEnable"] = 521;
  sim_data.signals.push_back(new dat_api<1>(&mod->memSPM_memModule_1__io_M_We));
  sim_data.signal_map["memSPM.memModule_1.io_M_We"] = 522;
  sim_data.signals.push_back(new dat_api<6>(&mod->memSPM_memModule_1__io_M_Addr));
  sim_data.signal_map["memSPM.memModule_1.io_M_Addr"] = 523;
  std::string memSPM_memModule_1__syncMem_path = "memSPM.memModule_1.syncMem";
  for (size_t i = 0 ; i < 256 ; i++) {
    sim_data.signals.push_back(new dat_api<8>(&mod->memSPM_memModule_1__syncMem.contents[i]));
    sim_data.signal_map[memSPM_memModule_1__syncMem_path+"["+itos(i,false)+"]"] = 524+i;
  }
  sim_data.signals.push_back(new dat_api<8>(&mod->memSPM_memModule__io_M_Data));
  sim_data.signal_map["memSPM.memModule.io_M_Data"] = 780;
  sim_data.signals.push_back(new dat_api<1>(&mod->memSPM_memModule__io_M_blockEnable));
  sim_data.signal_map["memSPM.memModule.io_M_blockEnable"] = 781;
  sim_data.signals.push_back(new dat_api<1>(&mod->memSPM_memModule__io_M_We));
  sim_data.signal_map["memSPM.memModule.io_M_We"] = 782;
  sim_data.signals.push_back(new dat_api<6>(&mod->memSPM_memModule__io_M_Addr));
  sim_data.signal_map["memSPM.memModule.io_M_Addr"] = 783;
  std::string memSPM_memModule__syncMem_path = "memSPM.memModule.syncMem";
  for (size_t i = 0 ; i < 256 ; i++) {
    sim_data.signals.push_back(new dat_api<8>(&mod->memSPM_memModule__syncMem.contents[i]));
    sim_data.signal_map[memSPM_memModule__syncMem_path+"["+itos(i,false)+"]"] = 784+i;
  }
  sim_data.signals.push_back(new dat_api<8>(&mod->memSPM_memModule__io_S_Data));
  sim_data.signal_map["memSPM.memModule.io_S_Data"] = 1040;
  sim_data.signals.push_back(new dat_api<8>(&mod->memSPM_memModule_1__io_S_Data));
  sim_data.signal_map["memSPM.memModule_1.io_S_Data"] = 1041;
  sim_data.signals.push_back(new dat_api<8>(&mod->memSPM_memModule_2__io_S_Data));
  sim_data.signal_map["memSPM.memModule_2.io_S_Data"] = 1042;
  sim_data.signals.push_back(new dat_api<8>(&mod->memSPM_memModule_3__io_S_Data));
  sim_data.signal_map["memSPM.memModule_3.io_S_Data"] = 1043;
  sim_data.clk_map["clk"] = new clk_api(&mod->clk);
}
