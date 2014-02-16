
public class ExternalDeviceTreeNode extends DeviceTreeNode {
	int clientId;

	public ExternalDeviceTreeNode(int clientId) {
		this.clientId = clientId;
	}

	public String toString() {
		return "External node " + clientId;
	}

	public int getClientId() {
		return clientId;
	}
}
