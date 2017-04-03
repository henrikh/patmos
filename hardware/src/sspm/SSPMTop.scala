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
    val ocp = Vec.fill(nConnectors) { new IODeviceIO().ocp }
  }
  val scheduler = Module(new Scheduler(nConnectors))
  val mem = Module(new memSPM(64))

  // Vector for each connector
  val connectors = Vec.fill(nConnectors) { Module(new SSPMConnector()).io }

  val decoder = UIntToOH(scheduler.io.out, nConnectors)

  // Connect the SSPMConnector with the SSPM
  for (j <- 0 until nConnectors) {
    connectors(j).ocp <> io.ocp(j)
    connectors(j).connectorSignals.S.Data := mem.io.S.Data

    connectors(j).connectorSignals.enable := decoder(j)
  }

  // Select which SSPMConnector has access to the memory
  mem.io.M.Data := connectors(scheduler.io.out).connectorSignals.M.Data
  mem.io.M.Addr := connectors(scheduler.io.out).connectorSignals.M.Addr
  mem.io.M.ByteEn := connectors(scheduler.io.out).connectorSignals.M.ByteEn
  connectors(scheduler.io.out).connectorSignals.S.Data := mem.io.S.Data
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
