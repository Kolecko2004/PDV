package cz.cvut.fel.agents.pdv.bfs;

import cz.cvut.fel.agents.pdv.demo.DummyMessage;
import cz.cvut.fel.agents.pdv.dsand.DSProcess;
import cz.cvut.fel.agents.pdv.dsand.Message;
import cz.cvut.fel.agents.pdv.bfs.pathMessage;
import java.util.List;
import java.util.Queue;
import java.util.function.BiConsumer;

public class BFSProcess extends DSProcess {

    /**
     * Seznam nasledniku uzlu spravovaneho aktualnim procesem
     */
    private List<String> neighbors;

    /**
     * Je uzel spravovany aktualnim procesem 'root' prohledavani?
     */
    private boolean  isRoot;

    /**
     * Je uzel spravovany aktualnim procesem cilovym uzlem?
     */
    private boolean isGoal;

    /**
     * Identifikator predchazejiciho procesu ('uzlu' na nejkratsi ceste)
     */
    private String predecessor = null;

    private long counter = 0;

    public BFSProcess(String id, Queue<Message> inbox, BiConsumer<String, Message> outbox,
                      List<String> neighbors, boolean isRoot, boolean isGoal) {
        super(id, inbox, outbox);
        this.neighbors = neighbors;
        this.isRoot = isRoot;
        this.isGoal = isGoal;

        // Pro jednoduchost nastavime predchazeji proces korene na koren samotny.
        if(isRoot) predecessor = id;
    }

    @Override
    public void act() {
        /*
            Doplnte algoritmus, ktery bude vykonavat kazdy proces

            1) Zpracujte prichozi zpravy v 'inbox'
            2) Vytvorte tridy pro posilane zpravy
            3) Doplnte logiku reakce na zpravu a odeslani novych zprav
         */

        System.out.printf("Agent %s has %d messages in his inbox and acts\n", id, inbox.size());
        if (isRoot && counter == 0){
            predecessor = id;

            for (int i = 0; i <= neighbors.size(); i++) {
                send(neighbors.get(i), new DummyMessage(counter++));
            }
        }

        while (!inbox.isEmpty()) {
            Message m = inbox.poll();

            if (m instanceof Message) {
                if (predecessor == null) {

                    if (isGoal) {
                        System.out.println("Goal was found sending messages to neighbours");

                        send(predecessor, new pathMessage("11"));
                    }

                    predecessor = m.sender;

                    for (int i = 0; i <= neighbors.size(); i++) {
                        send(neighbors.get(i), new DummyMessage(counter++));
                    }
                }

                else if (m instanceof pathMessage) {
                    send(predecessor, new pathMessage(id));
                }
            } else {
                System.out.println("Unknown message type: " + m.getClass().getName());
            }
        }
    }

    @Override
    public boolean isTerminated() {
        return false;
    }
}
