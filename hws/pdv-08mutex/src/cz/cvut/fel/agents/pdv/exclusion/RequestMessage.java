package cz.cvut.fel.agents.pdv.exclusion;

import cz.cvut.fel.agents.pdv.clocked.ClockedMessage;

public class RequestMessage extends ClockedMessage {
    public final String csName;
    public final String senderId;

    public RequestMessage(String csName, String senderId) {
        this.csName = csName;
        this.senderId = senderId; // Pokud toto chybí, je to instantní deadlock!
    }
}