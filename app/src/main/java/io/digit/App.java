package io.digit;

import io.digit.commands.CommandType;
import io.digit.sandbox.SandboxFactory;
import io.digit.sandbox.SandboxType;
import lombok.extern.slf4j.Slf4j;
import net.sourceforge.argparse4j.ArgumentParsers;
import net.sourceforge.argparse4j.inf.ArgumentParser;
import net.sourceforge.argparse4j.inf.ArgumentParserException;
import net.sourceforge.argparse4j.inf.Namespace;

import java.io.IOException;

@Slf4j
public class App {
    public static void main(String[] args) throws IOException, InterruptedException {
        ArgumentParser parser = ArgumentParsers.newFor("Sandboxing a third party JDBC driver").build()
                .defaultHelp(true)
                .description("Run SQLite JDBC driver with and without a sandbox");
        parser.addArgument("--command", "-c")
                .type(CommandType.class)
                .required(true)
                .help("The command you want to run");
        parser.addArgument("--sandbox", "-s")
                .type(SandboxType.class)
                .required(true)
                .help("Do you want to sandbox?");
        parser.addArgument("--iterations", "-i")
                .type(Integer.class)
                .setDefault(1)
                .help("How many times should we run the benchmark on this command?");

        log.info("Starting application");

        Namespace ns;
        try {
            ns = parser.parseArgs(args);
        } catch (ArgumentParserException e) {
            parser.handleError(e);
            throw new RuntimeException(e);
        }
        SandboxType sandboxType = ns.get("sandbox");
        String commandType = ns.getString("command");
        int iterations = ns.getInt("iterations");

        log.info("Running sandbox: {}, command: {}, iterations: {}", sandboxType, commandType, iterations);

        // Create the database connection; sandbox it in the appropriate way (or not at all)
        DatabaseRPC db = SandboxFactory.create(sandboxType).createDatabaseConnection();

        // Run the command that is being asked
        for (int i = 0; i < iterations; i++) {
            db.run(commandType);
        }

        log.info("Application ending");
    }
}
