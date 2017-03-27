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
 * Connection between SSPMConnector and the SSPM
 */
trait SSPMBackbone {
  val backbone = new Bundle() {
    val inbound = UInt(OUTPUT, 32)
    val outbound = UInt(INPUT, 32)
  }
}

/**
 * The basic SSPM
 */
class SPMemory extends Module() {
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

  // OCP pins and SSPMBackbone pins
  override val io = new CoreDeviceIO() with SSPMBackbone

  io.backbone.inbound := io.ocp.M.Data
  io.ocp.S.Data := io.backbone.outbound
}

/**
 * A top level of SSPM
 */
class SSPMTop(val nConnectors: Int) extends Module {
  val io = new Bundle {
    val in = Vec.fill(nConnectors) {UInt(INPUT, 32)}
    val out = UInt(OUTPUT, 32)
    val select = UInt(INPUT,
      // Ensure select statement has enough bits to address all connectors
      (scala.math.ceil(scala.math.log(nConnectors)/scala.math.log(2))).toInt)
  }

  // Vector for each connector
  val sspms = Vec.fill(nConnectors) { Module(new SSPMConnector()).io }

  // Connector the SSPMConnector with the SSPM
  for (j <- 0 until nConnectors) {
    sspms(j).ocp.M.Data := io.in(j)
    sspms(j).backbone.outbound := sspms(j).backbone.inbound
  }

  // Select which SSPMConnector has access to the SPM
  io.out := sspms(io.select).ocp.S.Data
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
