#ifndef __memModule__
#define __memModule__

#include "emulator.h"

class memModule_t : public mod_t {
 private:
  val_t __rand_seed;
  void __srand(val_t seed) { __rand_seed = seed; }
  val_t __rand_val() { return ::__rand_val(&__rand_seed); }
 public:
  dat_t<1> memModule__io_M_blockEnable;
  dat_t<1> memModule__io_M_We;
  dat_t<1> T2;
  dat_t<8> memModule__io_M_Data;
  dat_t<8> memModule__io_M_Addr;
  dat_t<8> memModule__dataReg;
  dat_t<8> memModule__io_S_Data;
  mem_t<8,256> memModule__syncMem;
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
class memModule_api_t : public emul_api_t {
 public:
  memModule_api_t(mod_t* m) : emul_api_t(m) { }
  void init_sim_data();
};

#endif
