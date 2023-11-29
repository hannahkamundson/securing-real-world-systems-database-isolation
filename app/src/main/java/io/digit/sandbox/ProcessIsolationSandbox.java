package io.digit.sandbox;

import io.digit.App;
import io.digit.DatabaseRPC;
import lombok.extern.slf4j.Slf4j;
import org.apache.xmlrpc.client.XmlRpcClient;
import org.apache.xmlrpc.client.XmlRpcClientConfigImpl;
import org.apache.xmlrpc.client.XmlRpcCommonsTransportFactory;
import org.apache.xmlrpc.client.util.ClientFactory;

import java.io.IOException;
import java.net.MalformedURLException;
import java.net.URL;

@Slf4j
public class ProcessIsolationSandbox implements SandboxCommand {
    @Override
    public DatabaseRPC createDatabaseConnection() throws IOException, InterruptedException {
        ProcessIsolationSandbox.createDatabaseProcess();

        // Create the RPC to communicate with the database process
        DatabaseRPC db = ProcessIsolationSandbox.createDatabaseRPC();

        // Wait for the process to be ready before starting in the next stage
        ProcessIsolationSandbox.waitUntilProcessServerReady(db);

        return db;
    }

    @Override
    public long getCpuStartTime(DatabaseRPC db) {
        return db.getProcessCpuTime();
    }

    @Override
    public long getCpuEndTime(DatabaseRPC db) {
        return db.getProcessCpuTime();
    }

    private static void waitUntilProcessServerReady(DatabaseRPC db) throws InterruptedException {
        boolean isReady = false;

        while (!isReady) {
            try {
                isReady = db.ready();
                log.debug("{}: The database process is ready", App.PID);
            } catch (Exception e) {
                log.debug("{}: Waiting for the database process to be ready", App.PID);

                // Ignore and give it 1 second to spin up before trying again
                Thread.sleep(1000);
            }
        }
    }

    private static DatabaseRPC createDatabaseRPC() throws MalformedURLException {
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

    private static void createDatabaseProcess() throws IOException {
        ProcessBuilder builder = new ProcessBuilder()
                // TODO: Don't hardcode the jar path
                .command("java", "-jar", "/home/hannah/Documents/Repos/securing-real-world-systems-database-isolation/db/target/db-1.0-SNAPSHOT.jar")
                .redirectError(ProcessBuilder.Redirect.INHERIT)
                .redirectInput(ProcessBuilder.Redirect.INHERIT)
                .redirectOutput(ProcessBuilder.Redirect.INHERIT);

        log.debug("{}: Starting process", App.PID);

        Process databaseProcess = builder.start();

        log.info("{}: Database process PID: {}", App.PID, databaseProcess.pid());

        // Make sure the child process is killed on shutdown
        Thread closeChildThread = new Thread(() -> {
            log.info("{}: Shutting down the database process", App.PID);
            databaseProcess.destroy();
        });

        Runtime.getRuntime().addShutdownHook(closeChildThread);

    }
}
