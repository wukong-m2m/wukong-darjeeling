import java.util.*;
import java.io.*;
import name.pachler.nio.file.*;

public class DirectoryWatcher extends Thread {
    private HashMap<WatchKey, DirectoryWatcherListener> watchkeys;
    private WatchService watchService;
    private String directory;

    public DirectoryWatcher(String directory) {
        this.directory = directory;
        watchkeys = new HashMap<WatchKey, DirectoryWatcherListener>();
        watchService = FileSystems.getDefault().newWatchService();
        start();
    }

    public void watchDirectory(String dir, DirectoryWatcherListener listener) {
        System.out.println("watching " + dir);
        Path watchedPath = Paths.get(dir);
        try {
            WatchKey key = watchedPath.register(watchService, StandardWatchEventKind.ENTRY_CREATE, StandardWatchEventKind.ENTRY_MODIFY, StandardWatchEventKind.ENTRY_DELETE);
            watchkeys.put(key, listener);
        } catch (UnsupportedOperationException e){
            System.err.println("file watching not supported!");
            System.err.println(e);
        }catch (IOException e){
            System.err.println("I/O errors");
            System.err.println(e);
        }
    }

    public void run() {
        while(true) {
            // take() will block until a file has been created/deleted
            WatchKey signalledKey;
            try {
                System.out.println("hallo2\n");
                signalledKey = watchService.take();
                System.out.println("hallo3\n");
                if (watchkeys.containsKey(signalledKey))
                    watchkeys.get(signalledKey).directoryChanged(signalledKey);
            } catch (InterruptedException ix){
                // we'll ignore being interrupted
                continue;
            } catch (ClosedWatchServiceException cwse){
                // other thread closed watch service
                System.out.println("watch service closed, terminating.");
                break;
            }
        }
    }
}