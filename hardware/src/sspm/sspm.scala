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

/**
 * This is a very basic ALU example.
 */
class SSPM extends CoreDevice() {

  override val io = new CoreDeviceIO()

  io.ocp.S.Data := io.ocp.M.Data
}

/**
 * A top level to wire FPGA buttons and LEDs
 * to the ALU input and output.
 */
class SSPMTop extends Module {
  val io = new Bundle {
    val in = UInt(INPUT, 32)
    val out = UInt(OUTPUT, 32)
  }

  val sspm = Module(new SSPM())

  // Map switches to the ALU input ports
  sspm.io.ocp.M.Data := io.in
  io.out := sspm.io.ocp.S.Data
}

// Generate the Verilog code by invoking chiselMain() in our main()
object SSPMMain {
  def main(args: Array[String]): Unit = {
    println("Generating the SSPM hardware")
    chiselMain(Array("--backend", "v", "--targetDir", "generated"),
      () => Module(new SSPMTop()))
  }
}

/**
 * Test the SSPM design
 */
class SSPMTester(dut: SSPM) extends Tester(dut) {

  poke(dut.io.ocp.M.Data, 42)
  step(1)
  expect(dut.io.ocp.S.Data, 42)
}

object SSPMTester {
  def main(args: Array[String]): Unit = {
    println("Testing the SSPM")
    chiselMainTest(Array("--genHarness", "--test", "--backend", "c",
      "--compile", "--targetDir", "generated"),
      () => Module(new SSPM())) {
        f => new SSPMTester(f)
      }
  }
}
