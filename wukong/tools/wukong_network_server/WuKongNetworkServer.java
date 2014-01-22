import java.net.*; 
import java.io.*; 
import java.util.*;

// protocol
// s->c [42]
// c->s 1 byte mode
// if mode==MODE_MESSAGE
//   c->s/s->c 1 byte length, 2 byte LE src 2 byte LE desr, length-5 bytes payload
// if mode==MODE_DISCOVERY
//   s->c      2 byte length, (length-2/2) times 2 byte LE client id

public class WuKongNetworkServer extends Thread
{
	protected static boolean serverContinue = true;
	protected static Map<Integer, WuKongNetworkServer> clients;
	protected Socket clientSocket;
	protected int clientId;
	protected Queue<byte[]> messages;
	protected boolean keepRunning;

	private final int MODE_MESSAGE = 1;
	private final int MODE_DISCOVERY = 2;

	public static void main(String[] args) throws IOException 
	{ 
		clients = new HashMap<Integer, WuKongNetworkServer>();

		ServerSocket serverSocket = null; 

		try { 
			serverSocket = new ServerSocket(10008); 
			System.out.println ("Connection Socket Created");
			try { 
				while (serverContinue) {
					serverSocket.setSoTimeout(10000);
					try {
						new WuKongNetworkServer (serverSocket.accept()); 
					}
					catch (SocketTimeoutException ste) {
//						System.out.println ("Timeout Occurred");
					}
				}
			} 
			catch (IOException e) { 
				System.err.println("Accept failed."); 
				System.exit(1); 
			} 
		} 
		catch (IOException e) { 
			System.err.println("Could not listen on port: 10008."); 
			System.exit(1); 
		} 
		finally {
			try {
				System.out.println ("Closing Server Connection Socket");
				serverSocket.close(); 
			}
			catch (IOException e) {
				System.err.println("Could not close port: 10008."); 
				System.exit(1); 
			} 
		}
	}

	private WuKongNetworkServer (Socket clientSoc)
	{
		messages = new LinkedList<byte[]>();
		keepRunning = true;
		clientSocket = clientSoc;
		start();
	}

	public void run()
	{
		try { 
			BufferedInputStream in = new BufferedInputStream(clientSocket.getInputStream());
			BufferedOutputStream out = new BufferedOutputStream(clientSocket.getOutputStream());

			// Say hi
			out.write(42);
			out.flush();

			// Get client mode
			int mode = in.read();

			if (mode == MODE_MESSAGE)
				handle_mode_message(in, out);
			else if (mode == MODE_DISCOVERY)
				handle_mode_discovery(in, out);
			else
				System.out.println("Unknown mode " + mode);

		} 
		catch (IOException e) { 
			System.err.println("Problem with Communication Server");
			System.err.println(e);
		} 
		finally {
			System.out.println("Node " + this.clientId + " disconnected.");
			if (WuKongNetworkServer.clients.get(this.clientId) == this)
				WuKongNetworkServer.clients.remove(this.clientId);			
		}
	}

	private void handle_mode_message(BufferedInputStream in, BufferedOutputStream out) throws IOException {
		long lastHeartbeat = 0;

		// Get client id
		this.clientId = in.read();
		if (this.clientId < 0)
			throw new IOException("No ID received");
		this.clientId += 256*in.read();
		System.out.println("New client " + this.clientId);


		if (WuKongNetworkServer.clients.get(this.clientId) != null)
			WuKongNetworkServer.clients.get(this.clientId).keepRunning = false; // Kill old thread for client with same ID if it was still around
		// Register this client in the global list
		WuKongNetworkServer.clients.put(this.clientId, this);

		while(keepRunning) {
			// Receive messages
			if (in.available() > 0) {
				byte length = (byte)in.read();
				byte [] message = new byte[length];
				message[0] = (byte)length;
				for (int i=1; i<length; i++)
					message[i] = (byte)in.read();
				int destId = message[3] + message[4]*256;

				System.out.print("Received message for " + destId + ", length " + length);

				WuKongNetworkServer destClient = WuKongNetworkServer.clients.get(destId);
				if (destClient != null) {
					destClient.messages.add(message);
					System.out.println("");
				}
				else
					System.out.println(" ---> dropped.");
			}

			// Send messages
			if (this.messages.size() > 0) {
				byte [] message = this.messages.poll();
				System.out.print("Forwarding message to node " + this.clientId);
				for (int i=0; i<message[0]; i++) {
					out.write(message[i]);
					System.out.print(" [" + String.format("%02X ", message[i]) + "]");
				}
				out.flush();
				System.out.println("");
			}

			// Heartbeat
			if (System.currentTimeMillis() - lastHeartbeat > 1000) {
				try {
					lastHeartbeat = System.currentTimeMillis();
					out.write(0);
					out.flush();
				}
				catch (IOException e){
					this.keepRunning = false;
				}
			}

			Thread.yield();
		}
	}

	private void handle_mode_discovery(BufferedInputStream in, BufferedOutputStream out) throws IOException {
		int number_of_clients = WuKongNetworkServer.clients.keySet().size();
		int length = 2+number_of_clients*2;
		System.out.println("discovery, number of clients " + number_of_clients);
		out.write((byte)length%256);
		out.write((byte)length/256);
		Integer[] ids = WuKongNetworkServer.clients.keySet().toArray(new Integer[0]);
		for (int i=0; i<number_of_clients; i++) {
			System.out.println("clients " + ids[i]);
			out.write((byte)(ids[i]%256));
			out.write((byte)(ids[i]/256));
		}
		out.flush();
	}
} 
