/*
 * Copyright: 2017, Technical University of Denmark, DTU Compute
 * Author: Henrik Enggaard Hansen (henrik.enggaard@gmail.com)
 * License: Simplified BSD License
 *
 * The core of a shared scratch-pad memory
 *
 * Based upon Martin Schoeberl's ALU example
 *
 */

package io

import Chisel._
import Node._

import patmos.Constants._

import ocp._

import sspm._

// /**
//  * Connection between SSPMConnector and the SSPMAegean
//  */
// trait SSPMConnectorSignals {
//   val connectorSignals = new Bundle() {
//     val enable = Bits(INPUT, 1)

//     val M = new Bundle() {
//        val Data = Bits(OUTPUT, DATA_WIDTH)
//        val Addr = Bits(OUTPUT, ADDR_WIDTH)
//        val ByteEn = Bits(OUTPUT, 4)
//        val WE = Bits(OUTPUT, 1)
//     }

//     val S = new Bundle() {
//        val Data = UInt(INPUT, DATA_WIDTH)
//     }
//   }
// }


/**
 * A top level of SSPMAegean
 */
class SSPMAegean(val nConnectors: Int) extends Module {

  //override val io = new CoreDeviceIO()

  val io = Vec.fill(nConnectors) { new OcpCoreSlavePort(ADDR_WIDTH, DATA_WIDTH) }

  // Generate modules
  val mem = Module(new memSPM(1024))
  val connectors = Vec.fill(nConnectors) { Module(new SSPMConnector()).io }
  val scheduler = Module(new Scheduler(nConnectors))
  val decoder = UIntToOH(scheduler.io.out, nConnectors)

  scheduler.io.done := Bool(true) // Set scheduler to start

  // Connect the SSPMConnector with the SSPMAegean
  for (j <- 0 until nConnectors) {
      connectors(j).ocp.M <> io(j).M
      connectors(j).ocp.S <> io(j).S
      connectors(j).connectorSignals.S.Data := mem.io.S.Data

    // Enable connectors based upon one-hot coding of scheduler
    connectors(j).connectorSignals.enable := decoder(j)
  } 

  mem.io.M.Data := connectors(scheduler.io.out).connectorSignals.M.Data
  mem.io.M.Addr := connectors(scheduler.io.out).connectorSignals.M.Addr
  mem.io.M.ByteEn := connectors(scheduler.io.out).connectorSignals.M.ByteEn
  mem.io.M.We := connectors(scheduler.io.out).connectorSignals.M.WE
}

// Generate the Verilog code by invoking chiselMain() in our main()
object SSPMAegeanMain {
  def main(args: Array[String]): Unit = {
    println("Generating the SSPMAegean hardware")
    val chiselArgs = args.slice(0,args.length) // If we later add for number of connectors,
                                               // we shoud index in another fashion, see SRamCtrl
    chiselMain(chiselArgs, () => Module(new SSPMAegean(4)))
  }
}


/**
 * Test the SSPMAegean design
 */
class SSPMAegeanTester(dut: SSPMAegean) extends Tester(dut) {
  def idle(core: Int) = {
    poke(dut.io(core).M.Cmd, OcpCmd.IDLE.litValue())
    poke(dut.io(core).M.Addr, 0)
    poke(dut.io(core).M.Data, 0)
    poke(dut.io(core).M.ByteEn, Bits("b0000").litValue())
  }

  def wr(addr: BigInt, data: BigInt, byteEn: BigInt, core: Int) = {
    poke(dut.io(core).M.Cmd, OcpCmd.WR.litValue())
    poke(dut.io(core).M.Addr, addr)
    poke(dut.io(core).M.Data, data)
    poke(dut.io(core).M.ByteEn, byteEn)
  }

  def rd(addr: BigInt, byteEn: BigInt, core: Int) = {
    poke(dut.io(core).M.Cmd, OcpCmd.RD.litValue())
    poke(dut.io(core).M.Addr, addr)
    poke(dut.io(core).M.Data, 0)
    poke(dut.io(core).M.ByteEn, byteEn)
  }

  def mem() = {
    peek(dut.mem.io.M.Data)
    peek(dut.mem.io.M.Addr)
    peek(dut.mem.io.M.We)
    peek(dut.mem.io.S.Data) 
  }  

  // Initial setup, all cores set to idle
  println("\nSetup initial state\n")

  for(i <- 0 until 4){
  	idle(i)
  }

  step(1)

  for(i <- 0 until 4){
  	expect(dut.io(i).S.Resp, 0)
  }  

  // Write test, write from core i to memory location,
  // stall, it should be so that the connectors store the command
  println("\nTest write\n")

  for(i <- 0 until 4){

	  wr(i*4, i+1, Bits("b1111").litValue(), i)  
    peek(dut.scheduler.io.out)
    mem()   

    step(1)    

  	  // Stall until slave response
	  while(peek(dut.io(i).S.Resp) != OcpResp.DVA.litValue()) {
      peek(dut.scheduler.io.out)
      mem()        
	    step(1)
	  }  

    // We do not wait an extra cycle, since the master can issue
    // a new command on the same cycle as it gets a response (timing diagram)
    // Request to read back the data to determine if correct
    rd(i*4, Bits("b1111").litValue(), i)    
    peek(dut.scheduler.io.out)
    mem()  

    step(1)

    // Stall until slave response
    while(peek(dut.io(i).S.Resp) != OcpResp.DVA.litValue()) {
      peek(dut.scheduler.io.out)      
      mem()     
      step(1)
    }      

    expect(dut.io(i).S.Data, i+1)   
    mem()    

    idle(i)
  }

  // Read  test
  println("\nRead test\n")

  for(i <- 0 until 4){
	  rd(i*4, Bits("b1111").litValue(), i)  	

    step(1)

  	// Stall until data valid
	  while(peek(dut.io(i).S.Resp) != OcpResp.DVA.litValue()) {
      peek(dut.scheduler.io.out)      
      mem() 
	    step(1)
	  }  	

    expect(dut.io(i).S.Data, i+1)       
    peek(dut.scheduler.io.out)      
    mem()
    idle(i)
  }  

}

object SSPMAegeanTester {
  def main(args: Array[String]): Unit = {
    println("Testing the SSPMAegean")
    chiselMainTest(Array("--genHarness", "--test", "--backend", "c",
      "--compile", "--targetDir", "generated"),
      () => Module(new SSPMAegean(4))) {
        f => new SSPMAegeanTester(f)
      }
  }
}


