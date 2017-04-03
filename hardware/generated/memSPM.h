#ifndef __memSPM__
#define __memSPM__

#include "emulator.h"

class memSPM_t : public mod_t {
 private:
  val_t __rand_seed;
  void __srand(val_t seed) { __rand_seed = seed; }
  val_t __rand_val() { return ::__rand_val(&__rand_seed); }
 public:
  dat_t<1> memSPM_memModule_3__io_M_blockEnable;
  dat_t<1> memSPM__io_M_We;
  dat_t<1> memSPM_memModule_3__io_M_We;
  dat_t<1> T4;
  dat_t<1> memSPM_memModule_2__io_M_blockEnable;
  dat_t<1> memSPM_memModule_2__io_M_We;
  dat_t<1> T11;
  dat_t<1> memSPM_memModule_1__io_M_blockEnable;
  dat_t<1> memSPM_memModule_1__io_M_We;
  dat_t<1> T18;
  dat_t<1> memSPM_memModule__io_M_blockEnable;
  dat_t<1> memSPM_memModule__io_M_We;
  dat_t<1> T25;
  dat_t<4> memSPM__io_M_ByteEn;
  dat_t<6> memSPM_memModule_3__io_M_Addr;
  dat_t<6> memSPM_memModule_2__io_M_Addr;
  dat_t<6> memSPM_memModule_1__io_M_Addr;
  dat_t<6> memSPM_memModule__io_M_Addr;
  dat_t<8> memSPM_memModule_3__io_M_Data;
  dat_t<8> memSPM__io_M_Addr;
  dat_t<8> T6;
  dat_t<8> memSPM_memModule_2__io_M_Data;
  dat_t<8> T13;
  dat_t<8> memSPM_memModule_1__io_M_Data;
  dat_t<8> T20;
  dat_t<8> memSPM_memModule__io_M_Data;
  dat_t<8> T27;
  dat_t<8> memSPM_memModule__io_S_Data;
  dat_t<8> memSPM_memModule_1__io_S_Data;
  dat_t<8> memSPM_memModule_2__io_S_Data;
  dat_t<8> memSPM_memModule_3__io_S_Data;
  dat_t<32> memSPM__io_M_Data;
  dat_t<32> memSPM__io_S_Data;
  mem_t<8,256> memSPM_memModule_3__syncMem;
  mem_t<8,256> memSPM_memModule_2__syncMem;
  mem_t<8,256> memSPM_memModule_1__syncMem;
  mem_t<8,256> memSPM_memModule__syncMem;
  clk_t clk;

  void init ( val_t rand_init = 0 );
  void clock_lo ( dat_t<1> reset, bool assert_fire=true );
  void clock_hi ( dat_t<1> reset );
  int clock ( dat_t<1> reset );
  void print ( FILE* f );
  void print ( std::ostream& s );
  void dump ( FILE* f, int t );
  void dump_init ( FILE* f );

};

#include "emul_api.h"
class memSPM_api_t : public emul_api_t {
 public:
  memSPM_api_t(mod_t* m) : emul_api_t(m) { }
  void init_sim_data();
};

#endif
