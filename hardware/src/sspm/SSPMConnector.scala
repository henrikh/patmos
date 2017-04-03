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

package sspm

import Chisel._

import patmos.Constants._

import ocp._

import io._

/**
 * The connector for each OCP bus
 */
class SSPMConnector extends IODevice() {

  // OCP pins and SSPMBackbone pins
  override val io = new IODeviceIO() with SSPMConnectorSignals

  val respReg = Reg(init = OcpResp.NULL)
  val cmdAcceptReg = Reg(init = Bits(width = 1))
  val writeEnableReg = Reg(init = Bits(0, width = 1))

  respReg := OcpResp.NULL
  cmdAcceptReg := Bits(0)

  when(io.connectorSignals.enable === Bits(1)) {
    when(io.ocp.M.Cmd === OcpCmd.RD || io.ocp.M.Cmd === OcpCmd.WR) {
      respReg := OcpResp.DVA
      cmdAcceptReg := Bits(1)
    }

    when(io.ocp.M.Cmd === OcpCmd.WR) {
      writeEnableReg := Bits(1)
    }
  }

  io.ocp.S.Resp := respReg
  io.ocp.S.CmdAccept := cmdAcceptReg

  io.connectorSignals.M.Data := io.ocp.M.Data
  io.connectorSignals.M.Addr := io.ocp.M.Addr
  io.connectorSignals.M.ByteEn := io.ocp.M.ByteEn
  io.connectorSignals.M.WE := writeEnableReg
  io.ocp.S.Data := io.connectorSignals.S.Data
}

// Generate the Verilog code by invoking chiselMain() in our main()
object SSPMConnectorMain {
  def main(args: Array[String]): Unit = {
    println("Generating the SSPM hardware")
    chiselMain(Array("--backend", "v", "--targetDir", "generated"),
      () => Module(new SSPMConnector()))
  }
}

/**
 * Test the SSPM design
 */
class SSPMConnectorTester(dut: SSPMConnector) extends Tester(dut) {

  // Write test
  //step(1)

  poke(dut.io.ocp.M.Cmd, OcpCmd.WR.litValue())
  poke(dut.io.ocp.M.Addr, 1)
  poke(dut.io.ocp.M.Data, 42)
  poke(dut.io.ocp.M.ByteEn, Bits("b1111").litValue())

  poke(dut.io.connectorSignals.enable, 1)

  step(1)

  expect(dut.io.connectorSignals.M.ByteEn, Bits("b1111").litValue())
  expect(dut.io.connectorSignals.M.Addr, 1)

  expect(dut.io.connectorSignals.M.Data, 42)

  expect(dut.io.ocp.S.Resp, 1)
  expect(dut.io.ocp.S.CmdAccept, 1)

  // Read test

  poke(dut.io.ocp.M.Cmd, OcpCmd.RD.litValue())
  poke(dut.io.ocp.M.Addr, 1)
  poke(dut.io.ocp.M.ByteEn, Bits("b1111").litValue())

  poke(dut.io.connectorSignals.enable, 1)

  step(1)

  expect(dut.io.connectorSignals.M.ByteEn, Bits("b1111").litValue())
  expect(dut.io.connectorSignals.M.Addr, 1)

  poke(dut.io.connectorSignals.S.Data, 42)

  expect(dut.io.ocp.S.Resp, 1)
  expect(dut.io.ocp.S.CmdAccept, 1)
  expect(dut.io.ocp.S.Data, 42)

  // Idle test
  step(1)

  poke(dut.io.ocp.M.Cmd, OcpCmd.IDLE.litValue())
  poke(dut.io.ocp.M.Addr, 1)
  poke(dut.io.ocp.M.ByteEn, Bits("b1111").litValue())

  poke(dut.io.connectorSignals.enable, 1)

  step(1)

  expect(dut.io.ocp.S.Resp, 0)
  expect(dut.io.ocp.S.CmdAccept, 0)
}

object SSPMConnectorTester {
  def main(args: Array[String]): Unit = {
    println("Testing the SSPM")
    chiselMainTest(Array("--genHarness", "--test", "--backend", "c",
      "--compile", "--targetDir", "generated"),
      () => Module(new SSPMConnector())) {
        f => new SSPMConnectorTester(f)
      }
  }
}
