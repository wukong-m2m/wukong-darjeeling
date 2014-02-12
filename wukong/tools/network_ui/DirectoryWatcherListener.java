import name.pachler.nio.file.*;

public interface DirectoryWatcherListener {
	void directoryChanged(WatchKey signalledKey);
}
