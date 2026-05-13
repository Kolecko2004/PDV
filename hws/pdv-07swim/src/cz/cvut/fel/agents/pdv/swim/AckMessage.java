package cz.cvut.fel.agents.pdv.swim;

import cz.cvut.fel.agents.pdv.dsand.Message;

public class AckMessage extends Message {
    public final String authorID;
    public AckMessage(String authorID) { this.authorID = authorID; }
}
