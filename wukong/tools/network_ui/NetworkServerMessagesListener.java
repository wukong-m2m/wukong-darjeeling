public interface NetworkServerMessagesListener {
	void messageDropped(int src, int dest, int[] message);
	void messageSent(int src, int dest, int[] message);
	void clientConnected(int client);
	void clientDisconnected(int client);
}

