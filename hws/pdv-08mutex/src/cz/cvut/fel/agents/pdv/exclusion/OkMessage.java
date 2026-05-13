package cz.cvut.fel.agents.pdv.exclusion;

import cz.cvut.fel.agents.pdv.clocked.ClockedMessage;

public class OkMessage extends ClockedMessage {
    public final String csName;

    public OkMessage(String csName) {
        this.csName = csName;
    }
}