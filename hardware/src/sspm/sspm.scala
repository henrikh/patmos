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

import ocp._

import io._

trait SSPMBackbone {
  val backbone = new Bundle() {
    val inbound = UInt(OUTPUT, 32)
    val outbound = UInt(INPUT, 32)
  }
}

/**
 * The basic SSPM
 */
class SSPMMemory extends Module() {
  val io = new Bundle {
    val in = UInt(OUTPUT, 32)
    val out = UInt(INPUT, 32)
  }

  io.out := io.in
}

/**
 * The connector for each OCP bus
 */
class SSPMConnector extends CoreDevice() {

  override val io = new CoreDeviceIO() with SSPMBackbone

  io.backbone.inbound := io.ocp.M.Data
  io.ocp.S.Data := io.backbone.outbound
}

/**
 * A top level to wire FPGA buttons and LEDs
 * to the ALU input and output.
 */
class SSPMTop(val nConnectors: Int) extends Module {
  val io = new Bundle {
    val in = Vec.fill(nConnectors) {UInt(INPUT, 32)}
    val out = Vec.fill(nConnectors) {UInt(OUTPUT, 32)}
  }

  val sspms = Vec.fill(nConnectors) { Module(new SSPMConnector()).io }

  for (j <- 0 until nConnectors) {
    sspms(j).ocp.M.Data := io.in(j)
    sspms(j).backbone.outbound := sspms(j).backbone.inbound
    io.out(j) := sspms(j).ocp.S.Data
  }
}

// Generate the Verilog code by invoking chiselMain() in our main()
object SSPMMain {
  def main(args: Array[String]): Unit = {
    println("Generating the SSPM hardware")
    chiselMain(Array("--backend", "v", "--targetDir", "generated"),
      () => Module(new SSPMTop(3)))
  }
}

/**
 * Test the SSPM design
 */
class SSPMTester(dut: SSPMTop) extends Tester(dut) {

  for (j <- 0 until 3) {
    poke(dut.io.in(j), j)
  }
  step(1)

  for (j <- 0 until 3) {
    poke(dut.io.select, j)
    step(1)
    expect(dut.io.out, j)
  }
}

object SSPMTester {
  def main(args: Array[String]): Unit = {
    println("Testing the SSPM")
    chiselMainTest(Array("--genHarness", "--test", "--backend", "c",
      "--compile", "--targetDir", "generated"),
      () => Module(new SSPMTop(3))) {
        f => new SSPMTester(f)
      }
  }
}
