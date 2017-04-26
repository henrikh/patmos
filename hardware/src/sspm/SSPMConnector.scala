/*
 * Copyright: 2017, Technical University of Denmark, DTU Compute
 * Author: Henrik Enggaard Hansen (henrik.enggaard@gmail.com)
 *         Andreas Toftegaard Kristensen (s144026@student.dtu.dk)
 * License: Simplified BSD License
 *
 * The controller for the access to the shared scratch-pad memory
 * for a Patmos CPU. The controller handles access to the
 * shared scratch-pad memory based on the schedulation and
 * commands from Patmos
 *
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
       val We = Bits(OUTPUT, 1)
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

  writeEnableReg := Bits(0)
  MAddrReg := Bits(0)
  MDataReg := Bits(0)
  MByteEnReg := Bits(0)
  respReg := OcpResp.NULL

  val s_idle :: s_waiting :: Nil = Enum(UInt(), 2)

  val state = Reg(init = s_idle)

  io.connectorSignals.M.Addr := MAddrReg
  io.connectorSignals.M.Data := MDataReg
  io.connectorSignals.M.ByteEn := MByteEnReg
  io.connectorSignals.M.We := writeEnableReg
  io.ocp.S.Resp := respReg
  io.ocp.S.Data := io.connectorSignals.S.Data

  // State machine description (mealy-style)

  when(state === s_idle) {

    when(io.ocp.M.Cmd === OcpCmd.RD || io.ocp.M.Cmd === OcpCmd.WR) {

      when(io.connectorSignals.enable === Bits(1)) {

        io.connectorSignals.M.Addr := io.ocp.M.Addr
        io.connectorSignals.M.Data := io.ocp.M.Data
        io.connectorSignals.M.ByteEn := io.ocp.M.ByteEn
        io.connectorSignals.M.We := io.ocp.M.Cmd(0)
        respReg := OcpResp.DVA

      }.otherwise{

        MAddrReg := io.ocp.M.Addr
        MByteEnReg := io.ocp.M.ByteEn
        MDataReg := io.ocp.M.Data
        writeEnableReg := io.ocp.M.Cmd(0)
        respReg := OcpResp.NULL

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
      io.connectorSignals.M.We := writeEnableReg
      respReg := OcpResp.DVA

      state := s_idle

    }.otherwise {

      state := s_waiting
    }

    MAddrReg := MAddrReg
    MDataReg := MDataReg
    MByteEnReg := MByteEnReg
    writeEnableReg := writeEnableReg
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

  def expectWr(addr: BigInt, data: BigInt, byteEn: BigInt, resp: BigInt) = {
      expect(dut.io.connectorSignals.M.Addr, addr)
      expect(dut.io.connectorSignals.M.Data, data)
      expect(dut.io.connectorSignals.M.ByteEn, byteEn)
      expect(dut.io.ocp.S.Resp, resp)
    }

  def expectRd(addr: BigInt, data: BigInt, byteEn: BigInt, resp: BigInt) = {
      expect(dut.io.connectorSignals.M.Addr, addr)
      expect(dut.io.connectorSignals.M.ByteEn, byteEn)
      expect(dut.io.ocp.S.Data, data)
      expect(dut.io.ocp.S.Resp, resp)
    }

  def rd(addr: BigInt, byteEn: BigInt) = {
    poke(dut.io.ocp.M.Cmd, OcpCmd.RD.litValue())
    poke(dut.io.ocp.M.Addr, addr)
    poke(dut.io.ocp.M.Data, 0)
    poke(dut.io.ocp.M.ByteEn, byteEn)
  }

  // Set to initial state

  println("\nSet to initial state\n")


  idle()
  poke(dut.io.connectorSignals.enable, 0)

  // Write test with delayed enable
  // expect that once enable comes on, it services the core

  step(1)

  println("\nWrite with delayed enable\n")


  wr(1, 42, Bits("b1111").litValue())

  step(1)

  idle()

  expectWr(1, 42, Bits("b1111").litValue(), 0)

  step(1)

  poke(dut.io.connectorSignals.enable, 1)

  expectWr(1, 42, Bits("b1111").litValue(), 0)

  step(1)

  // Core will now be serviced, so unless we
  // perform another operation, expect that
  // data, addr and byteEn become 0

  poke(dut.io.connectorSignals.enable, 0)
  idle()

  expectWr(1, 42, Bits("b1111").litValue(), 1)

  step(1)

  expectWr(0, 0, 0, 0)

  step(1)

  // Read test with delayed enable

  println("\nRead with delayed enable\n")

  poke(dut.io.connectorSignals.enable, 0)
  rd(1, Bits("b1111").litValue())

  step(1)

  idle()

  expectRd(1, 0, Bits("b1111").litValue(), 0)

  step(1)

  // Core will now be serviced, so unless we
  // perform another operation, expect that
  // Mdata, addr and byteEn become 0 in next cycle

  poke(dut.io.connectorSignals.enable, 1)
  poke(dut.io.connectorSignals.S.Data, 42)

  expectRd(1, 42, Bits("b1111").litValue(), 0)

  step(1)

  poke(dut.io.connectorSignals.enable, 0)

  expectRd(1, 42, Bits("b1111").litValue(), 1)

  // Write test with synchronous enable

  step(1)

  println("\nWrite test with synchronous enable\n")

  wr(1, 42, Bits("b1111").litValue())
  poke(dut.io.connectorSignals.enable, 1)

  expectWr(1, 42, Bits("b1111").litValue(), 0)

  step(1)

  idle()

  // If enable is off, expect 0

  poke(dut.io.connectorSignals.enable, 0)

  expectWr(0, 0, 0, 1)

  step(1)

  poke(dut.io.connectorSignals.enable, 1)

  expectWr(0, 0, 0, 0)

  step(1)

  poke(dut.io.connectorSignals.enable, 0)

  expectWr(0, 0, 0, 0)

  step(1)

  // Read test with synchronous enable

  step(1)

  println("\nRead test with synchronous enable\n")

  rd(1, Bits("b1111").litValue())
  poke(dut.io.connectorSignals.enable, 1)

  expectRd(1, 42, Bits("b1111").litValue(), 0)

  step(1)

  idle()

  poke(dut.io.connectorSignals.enable, 0)
  poke(dut.io.connectorSignals.S.Data, 42)

  expectRd(0, 42, 0, 1)

  step(1)

  poke(dut.io.connectorSignals.enable, 1)

  expectRd(0, 42, 0, 0)

  step(1)

  poke(dut.io.connectorSignals.enable, 0)
  poke(dut.io.connectorSignals.S.Data, 0)

  expectRd(0, 0, 0, 0)

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
