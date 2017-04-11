/*
 * An on-chip memory.
 *
 * Has input registers (without enable or reset).
 * Shall do byte enable.
 *
 * Size is in bytes.
 *
 * Authors: Andreas T. Kristensen
 */

package sspm

import Chisel._
import Node._

import patmos.Constants._

/*
 * A memory module
*/

class memModule(size: Int) extends Module {
  val io = new Bundle{

    val M = new Bundle() {
       val Data = UInt(INPUT, BYTE_WIDTH)
       val Addr = UInt(INPUT, log2Up(size / BYTES_PER_WORD)-2)
       val blockEnable = UInt(INPUT, 1) // From byte enable
       val We = UInt(INPUT, 1)    
    }

    val S = new Bundle() {
       val Data = UInt(OUTPUT, BYTE_WIDTH)
    }
  }

  // Second option is number of entries
  val syncMem = Mem(UInt(width=BYTE_WIDTH), size / BYTES_PER_WORD, seqRead=true)

  io.S.Data := syncMem(io.M.Addr)

  // Please note: the manual states that single-ported SRAMS can be inferred
  // when the read and write conditons are mutually exclusie in the same when chain.
  when(io.M.We === UInt(1) && io.M.blockEnable === UInt(1)) {
      syncMem(io.M.Addr) := io.M.Data

  }.otherwise{ 

    io.S.Data := syncMem(io.M.Addr) 
  }
}

// Generate the Verilog code by invoking chiselMain() in our main()
object memModuleMain {
  def main(args: Array[String]): Unit = {
    println("Generating the mem hardware")
    chiselMain(Array("--backend", "v", "--targetDir", "generated"),
      () => Module(new memModule(1024)))
  }
}

// Testing

class memModuleTester(dut: memModule) extends Tester(dut) {
  def wr(addr: BigInt, data: BigInt, blockEnable: BigInt) = {
    poke(dut.io.M.Data, data)
    poke(dut.io.M.Addr, addr)  
    poke(dut.io.M.blockEnable,  blockEnable)
    poke(dut.io.M.We, 1)      

    step(1)
  }

  def wr_test(data: BigInt) = {
    expect(dut.io.S.Data, data)
  }    

  // Write test
  wr(1, 42, 1)
  wr_test(42)

  // Write test where we chech that We does not affect read
  wr(1, 43, 1)
  // We take a step, and then We = 0
  poke(dut.io.M.We, 0)
  wr_test(43)  

  step(1)

  // Write test with block enable off
  wr(1, 13, 0)
  wr_test(43)

  // Write new value again
  wr(1, 66, 1)
  wr_test(66)
}

object memModuleTester {
  def main(args: Array[String]): Unit = {
    println("Testing the SSPM")
    chiselMainTest(Array("--genHarness", "--test", "--backend", "c",
      "--compile", "--targetDir", "generated"),
      () => Module(new memModule(1024))) {
        f => new memModuleTester(f)
      }
  }
}

/*
 * Memory module for scratchpad memory
 * note that we simply use ByteEn for We on the different ports.
*/ 

class  memSPM(size: Int) extends Module {
  val io = new Bundle{

    val M = new Bundle() {
       val Data = UInt(INPUT, DATA_WIDTH)
       val Addr = Bits(INPUT, log2Up(size / BYTES_PER_WORD))
       val ByteEn = UInt(INPUT, 4)
       val We = UInt(INPUT, 1)    
    }

    val S = new Bundle() {
       val Data = UInt(OUTPUT, DATA_WIDTH)
    }
  }

  val addrBits = log2Up(size / BYTES_PER_WORD) // Since we byte address, 

  // Vector for each connector
  val memories = Vec.fill(4) { Module(new memModule(size)).io } // Using .io here, means that we do not
                                                                // have to write e.g.  memories(j).io.M.Data
  val dataReg = Reg(init=UInt(0, width=BYTE_WIDTH))
  dataReg := UInt(0)
  io.S.Data := dataReg                                                                

  // Connect memories with the SSPM
  for (j <- 0 until 4) {
    memories(j).M.Data := io.M.Data((j+1)*BYTE_WIDTH-1, j*BYTE_WIDTH)
    memories(j).M.Addr := io.M.Addr(addrBits - 1, 2)
    memories(j).M.blockEnable := io.M.ByteEn(j)
    memories(j).M.We := io.M.We
  }  
  io.S.Data := Cat(memories(3).S.Data, memories(2).S.Data, memories(1).S.Data, memories(0).S.Data)
}

// Generate the Verilog code by invoking chiselMain() in our main()
object memSPMMain {
  def main(args: Array[String]): Unit = {
    println("Generating the mem hardware")
    chiselMain(Array("--backend", "v", "--targetDir", "generated"),
      () => Module(new memSPM(1024)))
  }
}

// Testing 

class memSPMTester(dut: memSPM) extends Tester(dut) {

  def wr(addr: BigInt, data: BigInt, byteEn: BigInt) = {
    poke(dut.io.M.Data, data)
    poke(dut.io.M.Addr, addr)  
    poke(dut.io.M.ByteEn,  byteEn)
    poke(dut.io.M.We, 1)      

    step(1)
  }

  def wr_test(data: BigInt) = {
    expect(dut.io.S.Data, data)
  }  

  // Write test
  wr(1, 42, Bits("b1111").litValue())
  wr_test(42)

  //step(1) We do not need these in between

  // Write test with block enable off
  wr(1, 13, 0)
  wr_test(42) // We expect the same data still

  // Write new value
  wr(1, 66, Bits("b1111").litValue())
  wr_test(66) 

  val g = peek(dut.io.S.Data)

  // Test byte writing 
  // First write all 1s to array, and selectively set these to 0 later on
  wr(0, Bits("hffffffff").litValue(), Bits("b1111").litValue())
  wr_test(Bits("hffffffff").litValue()) 

  wr(0, 0, Bits("b1000").litValue())
  wr_test(Bits("h00ffffff").litValue())

  wr(0, 0, Bits("b0100").litValue())
  wr_test(Bits("h0000ffff").litValue())

  wr(0, 0, Bits("b0010").litValue())
  wr_test(Bits("h000000ff").litValue())

  wr(0, 0, Bits("b0001").litValue())
  wr_test(Bits("h00000000").litValue())  

  // Test writing 2 bytes at the same time
  wr(0, Bits("hffffffff").litValue(), Bits("b1001").litValue())
  wr_test(Bits("hff0000ff").litValue())  
}


object memSPMTester {
  def main(args: Array[String]): Unit = {
    println("Testing the SSPM")
    chiselMainTest(Array("--genHarness", "--test", "--backend", "c",
      "--compile", "--targetDir", "generated"),
      () => Module(new memSPM(1024))) {
        f => new memSPMTester(f)
      }
  }
}
