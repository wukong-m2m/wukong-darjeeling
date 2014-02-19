import java.io.*;
import java.util.*;
import java.lang.Runtime;

public class ChildProcessManager {
    private HashMap<Process, String> childProcesses;

	public ChildProcessManager() {
		childProcesses = new HashMap<Process, String>();
		final ChildProcessManager final_this = this;

        // Add a hook to kill all child processes
        Runtime.getRuntime().addShutdownHook(new Thread(new Runnable() {
                public void run() {
                    for (Process p : final_this.childProcesses.keySet()) {
                        System.out.println("[" + final_this.childProcesses.get(p) + "] CHILD PROCESS TERMINATED.");
                        p.destroy();
                    }
                }
            }, "Shutdown-thread"));
	}

    public void forkChildProcess(final String name, final String command, final String directory) throws IOException {
        System.out.println("[" + name + "] starting " + command);
        final Process p = Runtime.getRuntime().exec(command, null, new File(directory));
        this.childProcesses.put(p, name);

        // Start a thread to monitor output
        Thread t = new Thread() {
            public void run() {
                System.out.println("[" + name + "] CHILD PROCESS STARTED.");
                BufferedReader in = new BufferedReader(new InputStreamReader(p.getInputStream()));  
                String line = null;
                try {
                    while ((line = in.readLine()) != null) {  
                        System.out.println("[" + name + "] " + line);
                    }
                    System.out.println("[" + name + "] CHILD PROCESS TERMINATED.");
                } catch (IOException e) {
                    System.err.println("Exception while reading output for child process " + name);
                    System.err.println(e);
                }
            }
        };
        t.start();
    }

	private class ChildProcessHandler {
		String commandline;
		String name;
		Integer nodeId;

		public ChildProcessHandler(String commandline, String name, Integer nodeId) {

		}
	}
}
