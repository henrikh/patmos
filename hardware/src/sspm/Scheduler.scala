/*
 * Copyright: 2017, Technical University of Denmark, DTU Compute
 * Author: Henrik Enggaard Hansen (henrik.enggaard@gmail.com)
 * License: Simplified BSD License
 *
 * The scheduler for a shared scratch-pad memory
 *
 * The scheduler takes the input-flag 'done' which specifies that the prior CPU is done and now wants to give access to the next CPU in the line.
 * The output is delayed with one clock cycle based on the register output.
 *
 *
 */

package sspm

import Chisel._
import patmos.Constants._
import ocp._

import io._

/**
 * A simple Scheduler, configurable counter which updates on done flag.
 */
class Scheduler(size: Int) extends Module {
  val io = new Bundle {
    val done = Bool(INPUT)
    val out = UInt(OUTPUT, size)
  }
  val r1 = Reg(init = UInt(0, log2Up(size)))

  when (io.done) {
    r1 := r1 + UInt(1)

    when ( r1 >= UInt(size - 1) ) {
      r1 := UInt(0)
    }
  }

  io.out := r1
}
// Generate the Verilog code by invoking chiselMain() in our main()
object SchedulerMain {
  def main(args: Array[String]): Unit = {
    println("Generating the Scheduler hardware")
    chiselMain(Array("--backend", "v", "--targetDir", "generated"),
      () => Module(new Scheduler(32)))
  }
}

/**
 * Test the Scheduler by printing out the value at each clock cycle.
 */
class SchedulerTester(dut: Scheduler, size: Int) extends Tester(dut) {
  // Testing that it counts to the size.
  poke(dut.io.done, true)
  for (i <- 0 until size) {
    expect(dut.io.out, i)
    step(1)
  }

  expect(dut.io.out, 0)

  // Disable progress
  poke(dut.io.done, false)

  step(1)
  expect(dut.io.out, 0)
}

/**
 * Create a Scheduler and a tester.
 */
object SchedulerTester {
  def main(args: Array[String]): Unit = {
    chiselMainTest(Array("--genHarness", "--test", "--backend", "c",
      "--compile", "--targetDir", "generated"),
      () => Module(new Scheduler(32))) {
        dut => new SchedulerTester(dut, 32)
      }
  }
}
