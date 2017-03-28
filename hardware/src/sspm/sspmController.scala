/*
 * Copyright: 2017, Technical University of Denmark, DTU Compute
 * Author: Henrik Enggaard Hansen (henrik.enggaard@gmail.com)
 * License: Simplified BSD License
 *
 * The controller used for each Patmos CPU connected to the shared scratchpad memory
 *
 * Based upon Martin Schoeberl's ALU example
 *
 */

package sspm

import Chisel._

import patmos.Constants._

import ocp._

import io._

/**
 * The basic SSPM on-chip memory. This should be based on Spm.Scala
 * however, for now it is just a dummy used for testing
 */
/*class SPMemory extends Module() {
  val io = new Bundle {
    val data_in = UInt(INPUT, 32) // From Patmos
    val addr = UInt(INPUT, 16)
    val data_out = UInt(OUTPUT, 32) // To Patmos
    val writeEnable = Bits(INPUT, 1)
  }
  io.out := io.in
}*/

/**
 * The extra inputs and the outputs to the SSPMConnector
 */
trait SSPMConnectorSignals {
  val connectorSignals = new Bundle() {
    val select = Bits(INPUT, 1)    // Select signal from either a round-robin or a scheduler to controller
    // Signals below are passed directly to SSPM 
    val data_in = UInt(INPUT, 32) // From SSPM    
    val data_out = UInt(OUTPUT, 32) // To SSPM
    val addr = UInt(OUTPUT, 16) // To SSPM
    val we = Bits(OUTPUT, 1) // TO SSPM
  }
}

/**
 * The controller for each OCP bus
 */
class SSPMController extends Module {
    val io = new Bundle {
      val MCmd = UInt(INPUT,3)
      val select = Bits(INPUT, 1)       
      val SResp = UInt(OUTPUT,2)
      //val SCmdAccept = UInt(OUTPUT,2)   
    }

  // Default responses
  val sRespReg = Reg(init = OcpResp.NULL)
  sRespReg := OcpResp.NULL  


  io.SResp := sRespReg

  //val scmdAcceptReg = Reg(init = OcpResp.NULL)
  //scmdAcceptReg := UInt(0)

  // We do not need a state machine for this controller 
  // if we can service a request we just do should
  // if not, the CPU simply stalls, and its command values
  // remain valid until we can service

  when(io.MCmd === OcpCmd.RD || io.MCmd === OcpCmd.WR) {

    when(io.select === UInt(1)) { // Service a request
      //scmdAcceptReg := UInt(1)
      sRespReg := OcpResp.DVA
    }
  }
}

/**
 * The connector for each OCP bus, this just passes data and addr, and contains the controller FSM
 */
class SSPMConnectorA extends CoreDevice() {

  // OCP pins and SSPMConnectorSignals pins
  override val io = new CoreDeviceIO() with SSPMConnectorSignals

  //Instantiate controller and connect 
  val sspmController = Module(new SSPMController())
  sspmController.io.MCmd := io.ocp.M.Cmd
  io.ocp.S.Resp := sspmController.io.SResp
  //sspmController.io.SCmdAccept := io.ocp.S.RespAccept 
  sspmController.io.select := io.connectorSignals.select 

  // Connect 
  io.connectorSignals.data_out := io.ocp.M.Data
  io.ocp.S.Data := io.connectorSignals.data_in
  io.connectorSignals.addr := io.ocp.M.Addr(16-1, 0)  // Index from 15 to 0
  io.connectorSignals.we := io.ocp.M.ByteEn(0) // This will be sustained to 1 by Patmos until serviced
                                       // and it will not affect SSPM since write is controlled by select (mux) and we (multiplexed value)
}

/**
 * SSPM CONNECTOR TEST SECTION
 */

// Generate the Verilog code by invoking chiselMain() in our main()
object SSPMConnectorMain {
  def main(args: Array[String]): Unit = {
    println("Generating the SSPM  connector hardware")
    chiselMain(Array("--backend", "v", "--targetDir", "generated"),
      () => Module(new SSPMConnectorA()))
  }
}

/**
 * Tester
 */
class SSPMConnectorTester(dut: SSPMConnectorA) extends Tester(dut) {
  // Peek
  peek(dut.io.ocp.S.Data)
  peek(dut.io.ocp.S.Resp)

  // Test


  // Write

  poke(dut.io.ocp.M.Data, 1)
  poke(dut.io.ocp.M.Cmd, 1) // Write
  poke(dut.io.ocp.M.ByteEn, 15)
  poke(dut.io.connectorSignals.select , 1)

  poke(dut.io.ocp.M.Addr, 0)      

  step(1)

  poke(dut.io.connectorSignals.data_in, 1) // From SSPM

  // Read

  // Not serviced

  step(1)
}

object SSPMConnectorTester {
  def main(args: Array[String]): Unit = {
    println("Testing the SSPM ConnectorA")
    chiselMainTest(Array("--genHarness", "--test", "--backend", "c",
      "--compile", "--vcd", "--targetDir", "generated"),
      () => Module(new SSPMConnectorA)) {
        f => new SSPMConnectorTester(f)
      }
  }
}