package cz.cvut.fel.agents.pdv.failure_detector;

import cz.cvut.fel.agents.pdv.dsand.DSProcess;
import cz.cvut.fel.agents.pdv.dsand.Message;
import cz.cvut.fel.agents.pdv.dsand.Pair;

import java.util.*;
import java.util.function.BiConsumer;

/**
 * Trida DetectorProcess. Ukolem kazde instance je detekovat ukoncene procesy pomoci all-to-all heartbeat algoritmu.
 * Krome implementace samotne metody act() budete pravdepodobne potrebovat zavest pomocne datove struktury.
 *
 * V metode act() mate napovedu k postupu.
 * Pokud si stale jeste nevite rady s frameworkem, inspiraci muzete nalezt v resenych prikladech ze cviceni.
 */
public class DetectorProcess extends DSProcess {

    // parametry posilani zprav
    // jak casto se maji zpravy posilat
    private final int period;
    // limit k prijeti heartbeatu
    private final int timeout;

    // interni pocitadlo, kolikrat byl proces vzbuzen
    // procesy se budi v kazdem kroku simulace vsechny
    // jejich wakeCounters tedy nemaji zadny drift
    private int wakeCount = 0;


    // TODO
    // doplnte vlastni datove struktury
    Map<String, Integer> processTime;
    Set<String> deadNode = new HashSet<>();
    int time;
    // vsechny procesy
    private final List<String> otherProcesses;

    public DetectorProcess(String id, Queue<Message> inbox, BiConsumer<String, Message> outbox,
                           int maxDelay, List<String> otherProcesses) {
        super(id, inbox, outbox);

        this.period = 10;
        this.otherProcesses = otherProcesses;

        // TODO
        // Nastavte timeout
        this.timeout = period + maxDelay;

        // TODO
        // inicializujte vlastni datove struktury
        processTime = new HashMap<>();
        for (String process : otherProcesses) {
            processTime.put(process, 0);
        }

        this.time = 0;
    }

    @Override
    public void act() {
        time++;


        // Muzete predpokladat, ze se zpravy neztraceji, jen zpozduji...
        // 1. zajistete, ze se s kazdou periodou poslou heartbeats vsem ostatnim procesum

        if (wakeCount % period == 0) {
            for (String node: otherProcesses) {
                if (!node.equals(id)) {
                    send(node, new HeartbeatMessage());
                }
            }
        }
        // 2. pokud obdrzite heartbeat od jineho procesu, aktualizujte si databazi zivych procesu
        while (!inbox.isEmpty()) {
            Message message = inbox.poll();
            processTime.put(message.sender, wakeCount);
        }

        // 3. detekujte havarovane procesy, takovy proces vypiste prave jednou
        for (String process : otherProcesses) {
            if (deadNode.contains(process) || process.equals(id)) {
                continue;
            }
            if (processTime.get(process) < wakeCount - timeout) {
                deadNode.add(process);
                System.out.println(id + ": Process " + process + " died.");
            }
        }

        // inkrementujeme citac probuzeni procesu
        wakeCount++;
    }
}
