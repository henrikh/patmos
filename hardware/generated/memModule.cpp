#include "memModule.h"

void memModule_t::init ( val_t rand_init ) {
  this->__srand(rand_init);
  memModule__syncMem.randomize(&__rand_seed);
  memModule__dataReg.randomize(&__rand_seed);
  clk.len = 1;
  clk.cnt = clk.len;
  clk.values[0] = 0;
}


int memModule_t::clock ( dat_t<1> reset ) {
  uint32_t min = ((uint32_t)1<<31)-1;
  if (clk.cnt < min) min = clk.cnt;
  clk.cnt-=min;
  if (clk.cnt == 0) clock_lo( reset );
  if (clk.cnt == 0) clock_hi( reset );
  if (clk.cnt == 0) clk.cnt = clk.len;
  return min;
}


void memModule_t::print ( FILE* f ) {
}
void memModule_t::print ( std::ostream& s ) {
}


void memModule_t::dump_init ( FILE* f ) {
}


void memModule_t::dump ( FILE* f, int t ) {
}




void memModule_t::clock_lo ( dat_t<1> reset, bool assert_fire ) {
  val_t T0;
  T0 = memModule__io_M_blockEnable.values[0] == 0x1L;
  val_t T1;
  T1 = memModule__io_M_We.values[0] == 0x1L;
  { T2.values[0] = T1 & T0;}
  val_t T3;
  { T3 = memModule__syncMem.get(memModule__io_M_Addr.values[0], 0);}
  val_t T4;
  { T4 = T2.values[0] ^ 0x1L;}
  val_t T5;
  { T5 = TERNARY_1(T4, T3, memModule__dataReg.values[0]);}
  { memModule__io_S_Data.values[0] = T5;}
}


void memModule_t::clock_hi ( dat_t<1> reset ) {
  { if (T2.values[0]) memModule__syncMem.put(memModule__io_M_Addr.values[0], 0, memModule__io_M_Data.values[0]);}
  dat_t<8> memModule__dataReg__shadow = 0x0;
  memModule__dataReg = 0x0;
}


void memModule_api_t::init_sim_data (  ) {
  sim_data.inputs.clear();
  sim_data.outputs.clear();
  sim_data.signals.clear();
  memModule_t* mod = dynamic_cast<memModule_t*>(module);
  assert(mod);
  sim_data.inputs.push_back(new dat_api<8>(&mod->memModule__io_M_Data));
  sim_data.inputs.push_back(new dat_api<8>(&mod->memModule__io_M_Addr));
  sim_data.inputs.push_back(new dat_api<1>(&mod->memModule__io_M_blockEnable));
  sim_data.inputs.push_back(new dat_api<1>(&mod->memModule__io_M_We));
  sim_data.outputs.push_back(new dat_api<8>(&mod->memModule__io_S_Data));
  std::string memModule__syncMem_path = "memModule.syncMem";
  for (size_t i = 0 ; i < 256 ; i++) {
    sim_data.signals.push_back(new dat_api<8>(&mod->memModule__syncMem.contents[i]));
    sim_data.signal_map[memModule__syncMem_path+"["+itos(i,false)+"]"] = 0+i;
  }
  sim_data.signals.push_back(new dat_api<8>(&mod->memModule__dataReg));
  sim_data.signal_map["memModule.dataReg"] = 256;
  sim_data.clk_map["clk"] = new clk_api(&mod->clk);
}
