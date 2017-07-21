/*
 * Copyright: 2017, Technical University of Denmark, DTU Compute
 * Author: Henrik Enggaard Hansen (henrik.enggaard@gmail.com)
 * License: Simplified BSD License
 *
 * A OCP allowing for single core testing of the SSPM.
 *
 * An instance of the SSPM core implemenation is used,
 * so the device behavior is identical to a multicore
 * usage.
 *
 * This implementation is suitable for emulation using
 * the C++ emulator.
 *
 */

package io

import Chisel._
import Node._

import patmos.Constants._

import ocp._

import sspm._

object SSPM extends DeviceObject {
  var nConnectors = 1

  def init(params: Map[String, String]) = {
    nConnectors = getPosIntParam(params, "nConnectors")
  }

  def create(params: Map[String, String]): SSPM = Module(new SSPM(nConnectors))

  trait Pins {}
}

/**
 * The basic SSPM
 */
class SPMemory extends Module() {
  val io = new Bundle {
    val in = UInt(OUTPUT, 32)
    val out = UInt(INPUT, 32)
    val writeEnable = Bits(INPUT, 1)
  }

  io.out := io.in
}

/**
 * A top level of SSPM
 */
class SSPM(val nConnectors: Int) extends CoreDevice {

  override val io = new CoreDeviceIO()

  // Connect one OCP interface from SSPM Aegean to the Patmos OCP
  Module(new SSPMAegean(nConnectors)).io(0) <> io.ocp
}

// Generate the Verilog code by invoking chiselMain() in our main()
object SSPMMain {
  def main(args: Array[String]): Unit = {
    println("Generating the SSPM hardware")
    val chiselArgs = args.slice(0,args.length) // If we later add for number of connectors,
                                               // we shoud index in another fashion, see SRamCtrl
    chiselMain(chiselArgs, () => Module(new SSPM(3)))
  }
}


/**
 * Test the SSPM design
 */
class SSPMTester(dut: SSPM) extends Tester(dut) {
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

  // Initial setup
  println("\nSetup initial state\n")

  idle()

  expect(dut.io.ocp.S.Resp, 0)

  // Write test
  println("\nTest write\n")
  step(1)

  wr(0xF00B0001L, 42, Bits("b1111").litValue())

  step(1)

  idle()

  println("\nStall until data valid\n")
  // Stall until data valid
  while(peek(dut.io.ocp.S.Resp) != OcpResp.DVA.litValue()) {
    step(1)
  }

  step(1)
  expect(dut.io.ocp.S.Resp, 0)

  // Read  test
  println("\nRead test\n")
  step(1)

  rd(0xF00B0001L, Bits("b1111").litValue())

  step(1)

  idle()

  println("\nStall until data valid\n")
  // Stall until data valid
  while(peek(dut.io.ocp.S.Resp) != OcpResp.DVA.litValue()) {
    step(1)
  }

  expect(dut.io.ocp.S.Data, 42)

  step(1)
  expect(dut.io.ocp.S.Resp, 0)

}

object SSPMTester {
  def main(args: Array[String]): Unit = {
    println("Testing the SSPM")
    chiselMainTest(Array("--genHarness", "--test", "--backend", "c",
      "--compile", "--targetDir", "generated"),
      () => Module(new SSPM(3))) {
        f => new SSPMTester(f)
      }
  }
}
