package io.digit;

import lombok.extern.slf4j.Slf4j;
import org.apache.xmlrpc.client.XmlRpcClient;
import org.apache.xmlrpc.client.XmlRpcClientConfigImpl;
import org.apache.xmlrpc.client.XmlRpcCommonsTransportFactory;
import org.apache.xmlrpc.client.util.ClientFactory;

import java.io.IOException;
import java.net.URL;

@Slf4j
public class App {
    public static void main(String[] args) throws IOException, InterruptedException {
        log.info("Starting application");

        // Create the process that will run the database and can be communicated with through RPC
        Process databaseProcess = App.createDatabaseProcess();

        // Create the RPC to communicate with the database process
        DatabaseRPC db = App.createDatabaseRPC();

        // Wait for the process to be ready before starting in the next stage
        boolean isReady = false;

        while (!isReady) {
            try {
                isReady = db.ready();
                log.info("The database process is ready");
            } catch (Exception e) {
                log.error("It isn't ready yet", e);
                // Ignore and just keep it false
                Thread.sleep(1000);
            }
        }

        log.info("Application ending");
    }

    private static DatabaseRPC createDatabaseRPC() {
        // Create the client that will interact with the process RPC
        XmlRpcClientConfigImpl config = new XmlRpcClientConfigImpl();
        // This is configured to the port the database process uses
        config.setServerURL(new URL("http://127.0.0.1:8080/xmlrpc"));
        config.setEnabledForExtensions(true);
        config.setConnectionTimeout(60 * 1000);
        config.setReplyTimeout(60 * 1000);

        XmlRpcClient client = new XmlRpcClient();

        // use Commons HttpClient as transport
        client.setTransportFactory(new XmlRpcCommonsTransportFactory(client));
        // set configuration
        client.setConfig(config);

        // make a call using dynamic proxy
        ClientFactory factory = new ClientFactory(client);
        return (DatabaseRPC) factory.newInstance(DatabaseRPC.class);
    }

    private static Process createDatabaseProcess() throws IOException {
        ProcessBuilder builder = new ProcessBuilder()
                // TODO: Don't hardcode the jar path
                .command("java", "-jar", "/home/hannah/Documents/Repos/securing-real-world-systems-database-isolation/db/target/db-1.0-SNAPSHOT.jar")
                .redirectError(ProcessBuilder.Redirect.INHERIT)
                .redirectInput(ProcessBuilder.Redirect.INHERIT)
                .redirectOutput(ProcessBuilder.Redirect.INHERIT);

        log.info("Starting process");

        Process databaseProcess = builder.start();

        log.info("Process PID: {}", databaseProcess.pid());

        // Make sure the child process is killed on shutdown
        Thread closeChildThread = new Thread(() -> {
            log.info("Shutting down the database process");
            databaseProcess.destroy();
        });

        Runtime.getRuntime().addShutdownHook(closeChildThread);

        return databaseProcess;
    }
}
