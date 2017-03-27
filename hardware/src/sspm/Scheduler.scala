/*
 * Copyright: 2017, Technical University of Denmark, DTU Compute
 * Author: Henrik Enggaard Hansen (henrik.enggaard@gmail.com)
 * License: Simplified BSD License
 *
 * The scheduler for a shared scratch-pad memory
 *
 * Based upon Martin Schoeberl's ALU example
 *
 */

package sspm

import Chisel._

import ocp._

import io._


/**
 * A top level of Scheduler
 */
class Scheduler() extends Module {
  val io = new Bundle {
    val in = UInt(INPUT, 32)
    val out = UInt(OUTPUT, 32)
  }

  io.out := io.in
}

// Generate the Verilog code by invoking chiselMain() in our main()
object SchedulerMain {
  def main(args: Array[String]): Unit = {
    println("Generating the SSPM hardware")
    chiselMain(Array("--backend", "v", "--targetDir", "generated"),
      () => Module(new Scheduler()))
  }
}

/**
 * Test the Scheduler design
 */
class SchedulerTester(dut: Scheduler) extends Tester(dut) {

  poke(dut.io.in, 42)
  step(1)
  expect(dut.io.out, 42)
}

object SchedulerTester {
  def main(args: Array[String]): Unit = {
    println("Testing the SSPM")
    chiselMainTest(Array("--genHarness", "--test", "--backend", "c",
      "--compile", "--targetDir", "generated"),
      () => Module(new Scheduler())) {
        f => new SchedulerTester(f)
      }
  }
}
