package io.digit;

import lombok.extern.slf4j.Slf4j;
import org.apache.xmlrpc.XmlRpcException;
import org.apache.xmlrpc.server.PropertyHandlerMapping;
import org.apache.xmlrpc.server.XmlRpcServer;
import org.apache.xmlrpc.server.XmlRpcServerConfigImpl;
import org.apache.xmlrpc.webserver.WebServer;

import java.io.IOException;

@Slf4j
public class DatabaseApp {
    private static final int PORT = 8080;

    public static void main(String[] args) throws XmlRpcException, IOException {
        log.info("Starting process");
        // Create the handler mapping to our class that handles it
        PropertyHandlerMapping propertyHandlerMapping = new PropertyHandlerMapping();
        // Create a new process factory, so we can maintain state across rpc calls
        DatabaseRPCImpl databaseRPC = new DatabaseRPCImpl();
        propertyHandlerMapping.setRequestProcessorFactoryFactory(new ProcessorFactoryFactory(databaseRPC));
        propertyHandlerMapping.setVoidMethodEnabled(true);
        propertyHandlerMapping.addHandler(DatabaseRPC.class.getName(), DatabaseRPC.class);

        // Start the server and add the handler mapping
        WebServer webServer = new WebServer(PORT);
        XmlRpcServer xmlRpcServer = webServer.getXmlRpcServer();

        xmlRpcServer.setHandlerMapping(propertyHandlerMapping);
        XmlRpcServerConfigImpl serverConfig = (XmlRpcServerConfigImpl) xmlRpcServer.getConfig();
        serverConfig.setEnabledForExtensions(true);
        serverConfig.setContentLengthOptional(false);

        log.info("Server starting");

        webServer.start();
    }
}
