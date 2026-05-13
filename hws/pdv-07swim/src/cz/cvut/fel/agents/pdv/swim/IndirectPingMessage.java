package cz.cvut.fel.agents.pdv.swim;

import cz.cvut.fel.agents.pdv.dsand.Message;

public class IndirectPingMessage extends Message {
    public final String targetToPing;
    public IndirectPingMessage(String targetToPing) { this.targetToPing = targetToPing; }
}
