package cz.cvut.fel.agents.pdv.exclusion;

import cz.cvut.fel.agents.pdv.clocked.ClockedMessage;
import cz.cvut.fel.agents.pdv.clocked.ClockedProcess;

import java.util.ArrayList;
import java.util.List;

public class ExclusionPrimitive {

    enum AcquisitionState {
        RELEASED,
        WANTED,
        HELD
    }

    private final ClockedProcess owner;
    private final String criticalSectionName;
    private final String[] allAccessingProcesses;
    private AcquisitionState state = AcquisitionState.RELEASED;
    private int requestTime = 0;
    private int pendingOks = 0;
    private final List<String> deferredRequests = new ArrayList<>();

    public ExclusionPrimitive(ClockedProcess owner,
                              String criticalSectionName,
                              String[] allAccessingProcesses) {
        this.owner = owner;
        this.criticalSectionName = criticalSectionName;
        this.allAccessingProcesses = allAccessingProcesses;
    }

    public void requestEnter() {
        state = AcquisitionState.WANTED;
        pendingOks = 0;

        owner.increaseTime();
        requestTime = owner.getTime();

        for (String pid : allAccessingProcesses) {
            if (!pid.equals(owner.id)) {
                owner.send(pid, new RequestMessage(criticalSectionName, owner.id));
                pendingOks++;
            }
        }

        if (pendingOks == 0) {
            state = AcquisitionState.HELD;
        }
    }

    public void exit() {
        state = AcquisitionState.RELEASED;

        for (String pid : deferredRequests) {
            owner.increaseTime();
            owner.send(pid, new OkMessage(criticalSectionName));
        }
        deferredRequests.clear();
    }

    public boolean accept(ClockedMessage m) {

        if (m instanceof RequestMessage) {
            RequestMessage req = (RequestMessage) m;

            if (!req.csName.equals(criticalSectionName)) {
                return false;
            }

            if (shouldDefer(req)) {
                deferredRequests.add(req.senderId);
            } else {
                owner.increaseTime();
                owner.send(req.senderId, new OkMessage(criticalSectionName));
            }
            return true;

        } else if (m instanceof OkMessage) {
            OkMessage ok = (OkMessage) m;

            if (!ok.csName.equals(criticalSectionName)) {
                return false;
            }

            if (state == AcquisitionState.WANTED) {
                pendingOks--;
                if (pendingOks == 0) {
                    state = AcquisitionState.HELD;
                }
            }
            return true;
        }

        return false;
    }

    private boolean shouldDefer(RequestMessage req) {
        if (state == AcquisitionState.HELD) {
            return true;
        }
        if (state == AcquisitionState.WANTED) {
            if (requestTime < req.sentOn) {
                return true;
            }
            if (requestTime == req.sentOn && owner.id.compareTo(req.senderId) < 0) {
                return true;
            }
        }
        return false;
    }

    public String getName() {
        return criticalSectionName;
    }

    public boolean isHeld() {
        return state == AcquisitionState.HELD;
    }
}