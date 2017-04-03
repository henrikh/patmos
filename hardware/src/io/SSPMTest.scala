/*
 * BRam interface for Patmos
 *
 * Authors: Luca Pezzarossa (lpez@dtu.dk)
 *
 */

package io

import Chisel._
import Node._
import ocp._

import sspm._

object SSPMTest extends DeviceObject {
  var extAddrWidth = 32
  var dataWidth = 32

  def init(params : Map[String, String]) = {
    extAddrWidth = getPosIntParam(params, "extAddrWidth")
    dataWidth = getPosIntParam(params, "dataWidth")
  }

  def create(params: Map[String, String]) : SSPMTest = {
    Module(new SSPMTest(extAddrWidth=extAddrWidth, dataWidth=dataWidth))
  }

  // In order to compile this properly, we apparently need a pin OUTPUT
  // this is however, just a test to see if it synthesises, so its a dummy
  trait Pins {
    val sSPMTestPins = new Bundle() {
      val MCmd = UInt(OUTPUT,3)

    }
  }  

}

class SSPMTest(extAddrWidth : Int = 32,
                     dataWidth : Int = 32) extends CoreDevice() {
  override val io = new CoreDeviceIO() with SSPMTest.Pins

  val SSPMMem = Module(new memSPM(1024)).io

  io.sSPMTestPins.MCmd := io.ocp.M.Data

  SSPMMem.M.Data := io.ocp.M.Data
  SSPMMem.M.Addr := io.ocp.M.Addr(extAddrWidth-1, 0)
  SSPMMem.M.ByteEn := io.ocp.M.ByteEn
  SSPMMem.M.We := io.ocp.M.Cmd(0)
  io.ocp.S.Data := SSPMMem.S.Data

  val respReg = Reg(init = OcpResp.NULL)
  respReg := OcpResp.NULL

  io.ocp.S.Resp := respReg

  when(io.ocp.M.Cmd === OcpCmd.WR || io.ocp.M.Cmd === OcpCmd.RD) {
    respReg := OcpResp.DVA
  }

}
