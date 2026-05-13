package cz.cvut.fel.agents.pdv.swim;

import cz.cvut.fel.agents.pdv.dsand.Message;
import cz.cvut.fel.agents.pdv.dsand.Pair;
import java.util.*;

public class ActStrategy {

    private final int maxDelay;
    private final List<String> aliveProcesses;

    private String currentlyChecking = null;
    private int stepCounter = 0;
    private int checkStartedAt = 0;
    private boolean receivedAck = false;
    private boolean indirectSent = false;

    private final Set<String> deadReported = new HashSet<>();

    private final Map<String, List<String>> requestsToForward = new HashMap<>();

    public ActStrategy(int maxDelayForMessages, List<String> otherProcesses,
                       int timeToDetectKilledProcess, int upperBoundOnMessages) {
        this.maxDelay = maxDelayForMessages;
        this.aliveProcesses = new ArrayList<>(otherProcesses);
        Collections.shuffle(this.aliveProcesses);
    }

    public List<Pair<String, Message>> act(Queue<Message> inbox, String disseminationProcess) {
        List<Pair<String, Message>> output = new ArrayList<>();
        stepCounter++;

        while (!inbox.isEmpty()) {
            Message msg = inbox.poll();

            if (msg instanceof DeadProcessMessage) {
                String dead = ((DeadProcessMessage) msg).getProcessID();
                aliveProcesses.remove(dead);
                requestsToForward.remove(dead);
                if (dead.equals(currentlyChecking)) currentlyChecking = null;

            } else if (msg instanceof PingMessage) {
                output.add(new Pair<>(msg.sender, new AckMessage(null)));

            } else if (msg instanceof IndirectPingMessage) {
                String target = ((IndirectPingMessage) msg).targetToPing;

                requestsToForward.computeIfAbsent(target, k -> new ArrayList<>()).add(msg.sender);

                output.add(new Pair<>(target, new PingMessage()));

            } else if (msg instanceof AckMessage) {
                AckMessage ack = (AckMessage) msg;
                String senderOfAck = msg.sender;

                if (currentlyChecking != null) {
                    if (senderOfAck.equals(currentlyChecking) ||
                            (ack.authorID != null && ack.authorID.equals(currentlyChecking))) {
                        receivedAck = true;
                    }
                }

                if (requestsToForward.containsKey(senderOfAck)) {
                    for (String originalRequester : requestsToForward.get(senderOfAck)) {
                        output.add(new Pair<>(originalRequester, new AckMessage(senderOfAck)));
                    }
                    requestsToForward.remove(senderOfAck);
                }
            }
        }

        if (currentlyChecking == null && !aliveProcesses.isEmpty()) {
            startNewCheck(output);
        } else if (currentlyChecking != null) {

            int timeoutDirect = 2 * maxDelay;
            int timeoutTotal = 7 * maxDelay;

            if (receivedAck) {
                currentlyChecking = null;
            } else if (!indirectSent && (stepCounter - checkStartedAt) > timeoutDirect) {
                sendIndirectPings(output, disseminationProcess);
                indirectSent = true;
            } else if ((stepCounter - checkStartedAt) > timeoutTotal) {
                if (!deadReported.contains(currentlyChecking)) {
                    output.add(new Pair<>(disseminationProcess, new PFDMessage(currentlyChecking)));
                    deadReported.add(currentlyChecking);
                }
                currentlyChecking = null;
            }
        }

        return output;
    }

    private void startNewCheck(List<Pair<String, Message>> output) {
        if (aliveProcesses.isEmpty()) return;

        currentlyChecking = aliveProcesses.remove(0);
        aliveProcesses.add(currentlyChecking);

        checkStartedAt = stepCounter;
        receivedAck = false;
        indirectSent = false;

        output.add(new Pair<>(currentlyChecking, new PingMessage()));
    }

    private void sendIndirectPings(List<Pair<String, Message>> output, String disseminationProcess) {
        int count = 0;
        List<String> candidates = new ArrayList<>(aliveProcesses);
        Collections.shuffle(candidates);

        for (String p : candidates) {
            if (!p.equals(currentlyChecking) && !p.equals(disseminationProcess) && count < 5) {
                output.add(new Pair<>(p, new IndirectPingMessage(currentlyChecking)));
                count++;
            }
        }
    }
}