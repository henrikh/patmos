/*
   Copyright 2013 Technical University of Denmark, DTU Compute.
   All rights reserved.

   This file is part of the time-predictable VLIW processor Patmos.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:

      1. Redistributions of source code must retain the above copyright notice,
         this list of conditions and the following disclaimer.

      2. Redistributions in binary form must reproduce the above copyright
         notice, this list of conditions and the following disclaimer in the
         documentation and/or other materials provided with the distribution.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER ``AS IS'' AND ANY EXPRESS
   OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
   OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN
   NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
   DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
   (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
   LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
   ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
   THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

   The views and conclusions contained in the software and documentation are
   those of the authors and should not be interpreted as representing official
   policies, either expressed or implied, of the copyright holder.
 */

/*
 * An on-chip memory.
 *
 * Has input registers (without enable or reset).
 * Shall do byte enable.
 * Output multiplexing and bit filling at the moment also here.
 * That might move out again when more than one memory is involved.
 *
 * Address decoding here. At the moment map to 0x00000000.
 * Only take care on a write.
 *
 * Size is in bytes.
 *
 * Authors: Martin Schoeberl (martin@jopdesign.com)
 *          Wolfgang Puffitsch (wpuffitsch@gmail.com)
 */

package sspm

import Chisel._
import Node._

class  memSPM extends Module {
  val io = new Bundle{

    val M = new Bundle() {
       val Data = UInt(INPUT, 32)
       val Addr = UInt(INPUT, 16)
       val ByteEn = UInt(INPUT, 4)
       val We = UInt(INPUT, 1)    
    }

    val S = new Bundle() {
       val Data = UInt(OUTPUT, 32)
    }
  }

  val syncMem = Mem(UInt(width=32), 128, seqRead=true)

  val dataReg = Reg(init=UInt(0, width=32))
  dataReg := UInt(0)
  io.S.Data := dataReg

  when(io.M.We === UInt(1)) {

    syncMem(io.M.Addr) := io.M.Data

  }.otherwise{

    io.S.Data := syncMem(io.M.Addr) 
  }
}

// Generate the Verilog code by invoking chiselMain() in our main()
object memSPMMain {
  def main(args: Array[String]): Unit = {
    println("Generating the mem hardware")
    chiselMain(Array("--backend", "v", "--targetDir", "generated"),
      () => Module(new memSPM))
  }
}