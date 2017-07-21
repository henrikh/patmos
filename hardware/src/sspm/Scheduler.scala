/*
 * Copyright: 2017, Technical University of Denmark, DTU Compute
 * Author: Henrik Enggaard Hansen (henrik.enggaard@gmail.com)
 *         Andreas Toftegaard Kristensen (s144026@student.dtu.dk)
 * License: Simplified BSD License
 *
 * The scheduler used in arbitration for a shared scratch-pad memory
 *
 * The scheduler takes the input-flag 'done' which specifies that the prior CPU is done and now wants to give access to the next CPU in the line.
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

  def lookahead(n:Int, waiting:Seq[Bool]):UInt = {
    val indices = (0 until waiting.length).map((x:Int) => UInt(x))
    PriorityMux(waiting.drop(n+1) ++ waiting.take(n+1), indices.drop(n+1) ++ indices.take(n+1))
  }

  val io = new Bundle {
    val done = Bool(INPUT)
    val out = UInt(OUTPUT)
    val lookahead = Vec.fill(size) { Bool(INPUT) }
  }
  val out = Reg(init = UInt(0, log2Up(size)))

  val lookaheaders = Vec((0 until size).map((x:Int) => lookahead(x, io.lookahead)))

  out := lookaheaders(out)

  io.out := out
}
// Generate the Verilog code by invoking chiselMain() in our main()
object SchedulerMain {
  def main(args: Array[String]): Unit = {
    println("Generating the Scheduler hardware")
    chiselMain(Array("--backend", "v", "--targetDir", "generated"),
      () => Module(new Scheduler(4)))
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

  // Check that it cycles back properly
  for (i <- 0 until size) {
    expect(dut.io.out, i)
    step(1)
   }  

  step(2)

  // Disable progress
  poke(dut.io.done, false)

  var schedulerOutput = 0 
      
  schedulerOutput = peek(dut.io.out).toInt  

  step(4)

  expect(dut.io.out, schedulerOutput)

  // Start progress again
  poke(dut.io.done, true)

  step(1)

  expect(dut.io.out, schedulerOutput+1)  


}

/**
 * Create a Scheduler and a tester.
 */
object SchedulerTester {
  def main(args: Array[String]): Unit = {
    chiselMainTest(Array("--genHarness", "--test", "--backend", "c",
      "--compile", "--targetDir", "generated"),
      () => Module(new Scheduler(4))) {
        dut => new SchedulerTester(dut, 4)
      }
  }
}
