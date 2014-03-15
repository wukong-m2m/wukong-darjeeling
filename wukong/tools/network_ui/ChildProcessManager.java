import javax.swing.*;
import java.awt.*;
import java.io.*;
import java.util.*;
import java.lang.Runtime;

public class ChildProcessManager {
    private ArrayList<ChildProcessHandler> childProcesses;
    private JTabbedPane logTabs;

	public ChildProcessManager() {
		this.childProcesses = new ArrayList<ChildProcessHandler>();
        this.logTabs = null;
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

    public void setLogTabbedPane(JTabbedPane logTabs) {
        this.logTabs = logTabs;
    }

    public void addChildProcess(String name, java.util.List<String> commandline, String directory, Integer clientId) {
        LogTextArea childLogTextArea = new LogTextArea();
        childLogTextArea.setEditable(false);
        if (this.logTabs != null)
            this.logTabs.addTab(name, childLogTextArea);
        this.childProcesses.add(new ChildProcessHandler(name, commandline, directory, clientId, childLogTextArea));
    }

	private class ChildProcessHandler {
		private java.util.List<String> commandline;
        private String arguments;
		private String directory;
		public String name;
		public Integer clientId;
        public boolean isRunning;
        private Process process;
        private TextArea textArea;

		public ChildProcessHandler(String name, java.util.List<String> commandline, String directory, Integer clientId, TextArea textArea) {
            this.commandline = commandline;
            this.arguments = arguments;
            this.directory = directory;
            this.name = name;
            this.clientId = clientId;
            this.isRunning = false;
            this.process = null;
            this.textArea = textArea;
            this.start();
		}

        public synchronized void start() {
            if (this.isRunning)
                return;

            // Start a thread to run the child process and monitor its output
            final ChildProcessHandler final_this = this;
            Thread t = new Thread() {
                public void run() {
                    try {
                        try {
                            System.out.println("[" + final_this.name + "] starting " + final_this.commandline.toString());
                            ProcessBuilder pb = new ProcessBuilder(final_this.commandline);
                            pb.directory(new File(final_this.directory));
                            pb.redirectErrorStream(true);
                            final_this.process = pb.start();
                        } catch (IOException e) {
                            System.err.println("Exception while starting child process: " + e);
                            System.err.println(commandline.get(0));
                            System.err.println("in");
                            System.err.println(directory);
                            final_this.process = null;
                        }

                        final_this.isRunning = true;
                        System.out.println("[" + name + "] CHILD PROCESS STARTED.");

                        BufferedReader in = new BufferedReader(new InputStreamReader(final_this.process.getInputStream(), "UTF-8"));  
                        String line = null;
                        while ((line = in.readLine()) != null) {
                            final String final_line = line;
                            javax.swing.SwingUtilities.invokeLater(new Runnable() {
                                public void run() {
                                    final_this.textArea.append(final_line + System.getProperty("line.separator"));
                                }
                            });
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
