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
 * Connection between SSPMConnector and the SSPM
 */
trait SSPMConnectorSignals {
  val connectorSignals = new Bundle() {
    val enable = Bits(INPUT, 1)

    val M = new Bundle() {
       val Data = Bits(OUTPUT, DATA_WIDTH)
       val Addr = Bits(OUTPUT, ADDR_WIDTH)
       val ByteEn = Bits(OUTPUT, 4)
       val WE = Bits(OUTPUT, 1)
    }

    val S = new Bundle() {
       val Data = UInt(INPUT, DATA_WIDTH)
    }
  }
}

/**
 * The connector for each OCP bus
 */
class SSPMConnector extends CoreDevice() {

  // OCP pins and SSPMBackbone pins
  override val io = new CoreDeviceIO() with SSPMConnectorSignals

  val respReg = Reg(init = OcpResp.NULL)
  val writeEnableReg = Reg(init = Bits(0, width = 1))
  val MAddrReg = Reg(init = Bits(0, width = ADDR_WIDTH))
  val MDataReg = Reg(init = Bits(0, width = DATA_WIDTH))
  val MByteEnReg = Reg(init = Bits(0, width = 4))

  respReg := OcpResp.NULL

  val s_idle :: s_waiting :: Nil = Enum(UInt(), 2)

  val state = Reg(init = s_idle)

  io.connectorSignals.M.Addr := Bits(0) 
  io.connectorSignals.M.Data := Bits(0) 
  io.connectorSignals.M.ByteEn := Bits(0) 
  io.connectorSignals.M.WE := Bits(0) 

  io.ocp.S.Resp := respReg
  io.ocp.S.Data := io.connectorSignals.S.Data  

  when(state === s_idle) {

    when(io.ocp.M.Cmd === OcpCmd.RD || io.ocp.M.Cmd === OcpCmd.WR) {

      when(io.connectorSignals.enable === Bits(1)) {

        io.connectorSignals.M.Addr := io.ocp.M.Addr
        io.connectorSignals.M.Data := io.ocp.M.Data
        io.connectorSignals.M.ByteEn := io.ocp.M.ByteEn
        io.connectorSignals.M.WE := io.ocp.M.Cmd(0)  
        respReg := OcpResp.DVA

      }.otherwise{

        MAddrReg := io.ocp.M.Addr
        MByteEnReg := io.ocp.M.ByteEn        
        MDataReg := io.ocp.M.Data
        writeEnableReg := io.ocp.M.Cmd(0)

        state := s_waiting

      }

    }.otherwise {

      state := s_idle

    }

  }

  when (state === s_waiting) {

    when(io.connectorSignals.enable === Bits(1)) {

      io.connectorSignals.M.Addr := MAddrReg
      io.connectorSignals.M.Data := MDataReg
      io.connectorSignals.M.ByteEn := MByteEnReg
      io.connectorSignals.M.WE := writeEnableReg
      respReg := OcpResp.DVA      

      state := s_idle      

      writeEnableReg := Bits(0)
      MAddrReg := Bits(0)
      MDataReg := Bits(0)
      MByteEnReg := Bits(0)

    }.otherwise {

      state := s_waiting

    }
  }
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
