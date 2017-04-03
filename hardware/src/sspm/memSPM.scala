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

  // Write test
  poke(dut.io.M.Data, 42)
  poke(dut.io.M.Addr, 1)  
  poke(dut.io.M.blockEnable, 1)
  poke(dut.io.M.We, 1)  

  step(1)

  //Read test

  poke(dut.io.M.We, 0)
  expect(dut.io.S.Data, 42)

  step(1)

  // Write test with block enable off
  poke(dut.io.M.blockEnable, 0)
  poke(dut.io.M.We, 1)    
  poke(dut.io.M.Data, 13)  

  step(1)

  //Read, expect no change

  poke(dut.io.M.We, 0)
  expect(dut.io.S.Data, 42)  

  step(1)

  // Write new value
  poke(dut.io.M.Data, 66)
  poke(dut.io.M.blockEnable, 1)
  poke(dut.io.M.We, 1)    

  step(1)

  // Expect 66  

  poke(dut.io.M.We, 0)
  expect(dut.io.S.Data, 66)   

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
    //This would be the preffered way, but Chisel3 does not support subword assignment for outputs?
    //io.S.Data((j+1)*8-1, j*8) := memories(j).S.Data // Replace 8 with BYTE_WIDTH?
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

  // Write test
  poke(dut.io.M.Data, 42)
  poke(dut.io.M.Addr, 1)  
  poke(dut.io.M.ByteEn,  Bits("b1111").litValue())
  poke(dut.io.M.We, 1)  

  step(1)

  //Read test

  poke(dut.io.M.We, 0)
  expect(dut.io.S.Data, 42)

  step(1)

  // Write test with block enable off
  poke(dut.io.M.ByteEn, 0)  
  poke(dut.io.M.We, 1)    
  poke(dut.io.M.Data, 13)  

  step(1)

  //Read, expect no change

  poke(dut.io.M.We, 0)
  expect(dut.io.S.Data, 42)  

  step(1)

  // Write new value
  poke(dut.io.M.Data, 66)
  poke(dut.io.M.ByteEn,  Bits("b1111").litValue())
  poke(dut.io.M.We, 1)    

  step(1)

  // Expect 66  

  poke(dut.io.M.We, 0)
  expect(dut.io.S.Data, 66)   

  step(1)

  // Test byte writing 
  // First write all 1s to array, and selectively set these to 0 later on
  poke(dut.io.M.Addr, 0)  
  poke(dut.io.M.Data, Bits("hffffffff").litValue())
  poke(dut.io.M.ByteEn,  Bits("b1111").litValue())
  poke(dut.io.M.We, 1)    

  step(1)  

  expect(dut.io.S.Data, Bits("hffffffff").litValue())    
  poke(dut.io.M.Data, 0)
  poke(dut.io.M.ByteEn,  Bits("b1000").litValue())

  step(1)

  expect(dut.io.S.Data, Bits("h00ffffff").litValue())  
  poke(dut.io.M.ByteEn,  Bits("b0100").litValue())

  step(1)

  expect(dut.io.S.Data, Bits("h0000ffff").litValue())  
  poke(dut.io.M.ByteEn,  Bits("b0010").litValue())

  step(1)

  expect(dut.io.S.Data, Bits("h000000ff").litValue())  
  poke(dut.io.M.ByteEn,  Bits("b001").litValue())

  step(1)

  expect(dut.io.S.Data, Bits("h00000000").litValue())  
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
