
public class ExternalDeviceTreeNode extends DeviceTreeNode {
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
