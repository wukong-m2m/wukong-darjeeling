import java.io.*;
import java.util.*;
import java.lang.Runtime;

public class ChildProcessManager {
    private ArrayList<ChildProcessHandler> childProcesses;

	public ChildProcessManager() {
		this.childProcesses = new ArrayList<ChildProcessHandler>();
		final ChildProcessManager final_this = this;

        // Add a hook to kill all child processes
        Runtime.getRuntime().addShutdownHook(new Thread(new Runnable() {
                public void run() {
                    for (ChildProcessHandler p : final_this.childProcesses) {
                        System.out.println("[" + p.name + "] CHILD PROCESS TERMINATED.");
                        p.stop();
                    }
                }
            }, "Shutdown-thread"));
	}

    public boolean hasChildProcess(Integer clientId) {
        for (ChildProcessHandler p : this.childProcesses) {
            if (p.clientId == clientId) {
                return true;
            }
        }
        return false;
    }
    public boolean isChildProcessRunning(Integer clientId) {
        for (ChildProcessHandler p : this.childProcesses) {
            if (p.clientId == clientId) {
                return p.isRunning;
            }
        }
        return false;
    }
    public void stopChildProcess(Integer clientId) {
        for (ChildProcessHandler p : this.childProcesses) {
            if (p.clientId == clientId) {
                p.stop();
            }
        }
    }
    public void startChildProcess(Integer clientId) {
        for (ChildProcessHandler p : this.childProcesses) {
            if (p.clientId == clientId) {
                p.start();
            }
        }        
    }

    public void addChildProcess(String name, String commandline, String directory, Integer clientId) {
        this.childProcesses.add(new ChildProcessHandler(name, commandline, directory, clientId));
    }

	private class ChildProcessHandler {
		private String commandline;
		private String directory;
		public String name;
		public Integer clientId;
        public boolean isRunning;
        private Process process;

		public ChildProcessHandler(String name, String commandline, String directory, Integer clientId) {
            this.commandline = commandline;
            this.directory = directory;
            this.name = name;
            this.clientId = clientId;
            this.isRunning = false;
            this.process = null;
            this.start();
		}

        public synchronized void start() {
            if (this.isRunning)
                return;

            try {
                System.out.println("[" + this.name + "] starting " + this.commandline);
                this.process = Runtime.getRuntime().exec(this.commandline, null, new File(this.directory));
            } catch (IOException e) {
                System.err.println("Exception while starting child process: " + e);
                System.err.println(commandline);
                System.err.println("in");
                System.err.println(directory);
                this.process = null;
            }

            this.isRunning = true;
            // Start a thread to monitor output
            final ChildProcessHandler final_this = this;
            Thread t = new Thread() {
                public void run() {
                    try {
                        System.out.println("[" + name + "] CHILD PROCESS STARTED.");
                        BufferedReader in = new BufferedReader(new InputStreamReader(final_this.process.getInputStream()));  
                        String line = null;
                        while ((line = in.readLine()) != null) {  
                            System.out.println("[" + name + "] " + line);
                        }
                    } catch (IOException e) {
                        System.err.println("Exception while reading output for child process " + name);
                        System.err.println(e);
                    } finally {
                        System.out.println("[" + name + "] CHILD PROCESS TERMINATED.");
                        final_this.isRunning = false;
                        final_this.process = null;
                    }
                }
            };
            t.start();
        }

        public void stop() {
            if (this.process != null)
                this.process.destroy();
        }
	}
}
