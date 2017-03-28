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
 * The connector for each OCP bus
 */
class SSPMConnector extends IODevice() {

  // OCP pins and SSPMBackbone pins
  override val io = new IODeviceIO() with SSPMConnectorSignals

  io.connectorSignals.M.Data := io.ocp.M.Data
  io.ocp.S.Data := io.connectorSignals.S.Data
}
