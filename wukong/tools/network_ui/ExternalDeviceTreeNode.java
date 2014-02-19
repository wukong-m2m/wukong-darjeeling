
public class ExternalDeviceTreeNode extends DeviceTreeNode {
	// This class represents a network client without an IO filesystem in the network directory.
	// This chould either be the master, or for instance a real Galileo node joining the simulated network.
	int clientId;

	public ExternalDeviceTreeNode(int clientId) {
		this.clientId = clientId;
	}

	public String toString() {
		if (this.clientId == 1) // Assuming the master always has id 1
			return "Master";
		else
			return "External node " + this.clientId;
	}

	public int getClientId() {
		return clientId;
	}
}
