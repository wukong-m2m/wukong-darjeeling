
public class ExternalDeviceTreeNode extends DeviceTreeNode {
	int clientId;

	public ExternalDeviceTreeNode(int clientId) {
		this.clientId = clientId;
	}

	public String toString() {
		return "node_" + clientId;
	}

	public int getClientId() {
		return clientId;
	}
}
