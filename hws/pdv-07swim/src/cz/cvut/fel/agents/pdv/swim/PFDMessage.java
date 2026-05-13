package cz.cvut.fel.agents.pdv.swim;

import cz.cvut.fel.agents.pdv.dsand.Message;

public class PFDMessage extends Message {

  private final String detectedProcess;

  public PFDMessage(String detectedProcess) {
    this.detectedProcess = detectedProcess;
  }

  public String getDetectedProcess() {
    return detectedProcess;
  }
}
