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
class SSPMConnector extends CoreDevice() {

  // OCP pins and SSPMBackbone pins
  override val io = new CoreDeviceIO() with SSPMConnectorSignals

  val respReg = Reg(init = OcpResp.NULL)
  val writeEnableReg = Reg(init = Bits(0, width = 1))
  val MAddrReg = Reg(init = Bits(width = ADDR_WIDTH))
  val MDataReg = Reg(init = Bits(width = DATA_WIDTH))
  //val SDataReg = Reg(init = Bits(width = DATA_WIDTH))
  val MByteEnReg = Reg(init = Bits(width = 4))
  val armedReg = Reg(init = Bits(0, width = 1))

  respReg := OcpResp.NULL

  when(io.ocp.M.Cmd === OcpCmd.RD || io.ocp.M.Cmd === OcpCmd.WR) {
    MAddrReg := io.ocp.M.Addr
    MByteEnReg := io.ocp.M.ByteEn
    armedReg := Bits(1)
  }

  when(io.ocp.M.Cmd === OcpCmd.WR) {
    MDataReg := io.ocp.M.Data
    writeEnableReg := Bits(1)
  }

  when(io.connectorSignals.enable === Bits(1)) {

    when(armedReg === Bits(1)) {
      respReg := OcpResp.DVA
      armedReg := Bits(0)
      writeEnableReg := Bits(0)
      MAddrReg := Bits(0)
      MDataReg := Bits(0)
      MByteEnReg := Bits(0)
    }
  }

  io.ocp.S.Resp := respReg

  io.connectorSignals.M.Addr := MAddrReg
  io.connectorSignals.M.Data := MDataReg
  io.connectorSignals.M.ByteEn := MByteEnReg
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
  def idle() = {
    poke(dut.io.ocp.M.Cmd, OcpCmd.IDLE.litValue())
    poke(dut.io.ocp.M.Addr, 0)
    poke(dut.io.ocp.M.Data, 0)
    poke(dut.io.ocp.M.ByteEn, Bits("b0000").litValue())
  }

  def wr(addr: BigInt, data: BigInt, byteEn: BigInt) = {
    poke(dut.io.ocp.M.Cmd, OcpCmd.WR.litValue())
    poke(dut.io.ocp.M.Addr, addr)
    poke(dut.io.ocp.M.Data, data)
    poke(dut.io.ocp.M.ByteEn, byteEn)
  }

  def rd(addr: BigInt, byteEn: BigInt) = {
    poke(dut.io.ocp.M.Cmd, OcpCmd.RD.litValue())
    poke(dut.io.ocp.M.Addr, addr)
    poke(dut.io.ocp.M.Data, 0)
    poke(dut.io.ocp.M.ByteEn, byteEn)
  }

  idle()
  poke(dut.io.connectorSignals.enable, 0)

  // Write test with delayed enable

  step(1)

  wr(1, 42, Bits("b1111").litValue())

  step(1)

  idle()

  expect(dut.io.connectorSignals.M.Addr, 1)
  expect(dut.io.connectorSignals.M.Data, 42)
  expect(dut.io.connectorSignals.M.ByteEn, Bits("b1111").litValue())
  expect(dut.io.ocp.S.Resp, 0)

  step(1)

  expect(dut.io.connectorSignals.M.Addr, 1)
  expect(dut.io.connectorSignals.M.Data, 42)
  expect(dut.io.connectorSignals.M.ByteEn, Bits("b1111").litValue())
  expect(dut.io.ocp.S.Resp, 0)

  poke(dut.io.connectorSignals.enable, 1)

  step(1)
  poke(dut.io.connectorSignals.enable, 0)

  expect(dut.io.ocp.S.Resp, 1)

  idle()

  step(1)
  expect(dut.io.ocp.S.Resp, 0)

  step(1)

  // Read test with delayed enable

  poke(dut.io.connectorSignals.enable, 0)
  rd(1, Bits("b1111").litValue())

  step(1)
  idle()

  expect(dut.io.ocp.S.Resp, 0)
  expect(dut.io.connectorSignals.M.ByteEn, Bits("b1111").litValue())
  expect(dut.io.connectorSignals.M.Addr, 1)

  step(1)
  expect(dut.io.ocp.S.Resp, 0)
  expect(dut.io.connectorSignals.M.ByteEn, Bits("b1111").litValue())
  expect(dut.io.connectorSignals.M.Addr, 1)

  poke(dut.io.connectorSignals.enable, 1)
  poke(dut.io.connectorSignals.S.Data, 42)

  step(1)
  poke(dut.io.connectorSignals.enable, 0)
  poke(dut.io.connectorSignals.S.Data, 0)

  expect(dut.io.ocp.S.Resp, 1)
  expect(dut.io.ocp.S.Data, 42)

  // Write test with synchronous enable

  step(1)

  wr(1, 42, Bits("b1111").litValue())
  poke(dut.io.connectorSignals.enable, 1)

  expect(dut.io.connectorSignals.M.Addr, 0)
  expect(dut.io.connectorSignals.M.Data, 0)
  expect(dut.io.connectorSignals.M.ByteEn, Bits("b0000").litValue())

  peek(dut.io.ocp.S.Resp)

  step(1)
  idle()

  poke(dut.io.connectorSignals.enable, 0)

  expect(dut.io.connectorSignals.M.Addr, 1)
  expect(dut.io.connectorSignals.M.Data, 42)
  expect(dut.io.connectorSignals.M.ByteEn, Bits("b1111").litValue())

  step(1)

  poke(dut.io.connectorSignals.enable, 1)

  expect(dut.io.connectorSignals.M.Addr, 1)
  expect(dut.io.connectorSignals.M.Data, 42)
  expect(dut.io.connectorSignals.M.ByteEn, Bits("b1111").litValue())

  step(1)
  poke(dut.io.connectorSignals.enable, 0)

  expect(dut.io.ocp.S.Resp, 1)

  step(1)

  idle()

  // Write test with synchronous enable

  step(1)

  rd(1, Bits("b1111").litValue())
  poke(dut.io.connectorSignals.enable, 1)

  expect(dut.io.connectorSignals.M.Addr, 0)
  expect(dut.io.connectorSignals.M.ByteEn, Bits("b0000").litValue())

  peek(dut.io.ocp.S.Resp)

  step(1)
  idle()

  poke(dut.io.connectorSignals.enable, 0)

  expect(dut.io.connectorSignals.M.Addr, 1)
  expect(dut.io.connectorSignals.M.ByteEn, Bits("b1111").litValue())

  step(1)

  poke(dut.io.connectorSignals.enable, 1)

  expect(dut.io.connectorSignals.M.Addr, 1)
  expect(dut.io.connectorSignals.M.ByteEn, Bits("b1111").litValue())

  poke(dut.io.connectorSignals.S.Data, 42)

  step(1)
  poke(dut.io.connectorSignals.enable, 0)
  poke(dut.io.connectorSignals.S.Data, 0)

  expect(dut.io.ocp.S.Resp, 1)
  expect(dut.io.ocp.S.Data, 42)

  step(1)

  idle()

}

object SSPMConnectorTester {
  def main(args: Array[String]): Unit = {
    println("Testing the SSPM")
    chiselMainTest(Array("--genHarness", "--test", "--backend", "c",
      "--compile", "--targetDir", "generated", "--vcd"),
      () => Module(new SSPMConnector())) {
        f => new SSPMConnectorTester(f)
      }
  }
}
