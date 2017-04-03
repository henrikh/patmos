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
    }

    val S = new Bundle() {
       val Data = UInt(INPUT, DATA_WIDTH)
    }
  }
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
class SSPMTop(val nConnectors: Int) extends Module {
  val io = new Bundle {
    val in = Vec.fill(nConnectors) {UInt(INPUT, DATA_WIDTH)}
    val ocp = Vec.fill(nConnectors) { new IODeviceIO().ocp }
    val out = UInt(OUTPUT, DATA_WIDTH)
    val select = UInt(INPUT, log2Up(nConnectors))
  }

  // Vector for each connector
  val sspms = Vec.fill(nConnectors) { Module(new SSPMConnector()).io }

  // Connect the SSPMConnector with the SSPM
  for (j <- 0 until nConnectors) {
    sspms(j).ocp.M.Data := io.in(j)
    sspms(j).connectorSignals.S.Data := sspms(j).connectorSignals.M.Data
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
