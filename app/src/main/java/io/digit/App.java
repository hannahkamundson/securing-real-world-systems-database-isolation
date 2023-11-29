package io.digit;

import io.digit.commands.Command;
import io.digit.commands.CommandFactory;
import io.digit.commands.CommandType;
import io.digit.sandbox.SandboxCommand;
import io.digit.sandbox.SandboxFactory;
import io.digit.sandbox.SandboxType;
import lombok.extern.slf4j.Slf4j;
import net.sourceforge.argparse4j.ArgumentParsers;
import net.sourceforge.argparse4j.inf.ArgumentParser;
import net.sourceforge.argparse4j.inf.ArgumentParserException;
import net.sourceforge.argparse4j.inf.Namespace;

import java.io.IOException;
import java.util.Comparator;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

@Slf4j
public class App {
    public static final long PID = ProcessHandle.current().pid();
    public static void main(String[] args) throws IOException, InterruptedException {
        ArgumentParser parser = ArgumentParsers.newFor("Sandboxing a third party JDBC driver").build()
                .defaultHelp(true)
                .description("Run SQLite JDBC driver with and without a sandbox");
        parser.addArgument("--command", "-c")
                .type(CommandType.class)
                .nargs("+")
                .setDefault(List.of(
                        // This order shouldn't cause any problems
                        CommandType.CONNECTION,
                        CommandType.CREATE_TABLE,
                        CommandType.INSERT,
                        CommandType.SELECT,
                        CommandType.DELETE,
                        CommandType.DELETE_TABLE
                ))
                .help("The command you want to run");
        parser.addArgument("--sandbox", "-s")
                .type(SandboxType.class)
                .required(true)
                .help("Do you want to sandbox?");
        parser.addArgument("--iterations", "-i")
                .type(Integer.class)
                .setDefault(20)
                .help("How many times should we run the benchmark on this command?");

        log.info("{}: Main application PID: {}", App.PID, App.PID);
        log.info("{}: Starting application", App.PID);

        Namespace ns;
        try {
            ns = parser.parseArgs(args);
        } catch (ArgumentParserException e) {
            parser.handleError(e);
            throw new RuntimeException(e);
        }
        SandboxType sandboxType = ns.get("sandbox");
        List<CommandType> commandTypes = ns.get("command");
        int iterations = ns.getInt("iterations");

        SandboxCommand sandboxCommand = SandboxFactory.create(sandboxType);
        DatabaseRPC db = sandboxCommand.createDatabaseConnection();

        Map<CommandType, ExecutionResults> resultsMap = new HashMap<>();

        for (CommandType commandType: commandTypes) {
            log.info("{}: Running sandbox: {}, command: {}, iterations: {}", App.PID, sandboxType, commandType, iterations);

            ExecutionResults.Builder builder = ExecutionResults.builder();

            Command<?> command = CommandFactory.create(commandType);
            builder.startTime();
            builder.cpuStartTime();
            builder.otherCpuStartTime(sandboxCommand.getCpuStartTime(db));

            // Run the command that is being asked
            for (int i = 1; i <= iterations; i++) {
                Object value = db.run(command, i);
                log.debug("{}: Outputting results {}", App.PID, command.interpretResults(value));
            }

            builder.cpuEndTime();
            builder.otherCpuEndTime(sandboxCommand.getCpuEndTime(db));
            builder.endTime();

            resultsMap.put(commandType, builder.build());
        }

        log.debug("{}: Application ending", App.PID);

        StringBuilder stringBuilder = new StringBuilder();
        stringBuilder.append('\n');
        stringBuilder.append('\n');
        stringBuilder.append("SANDBOX TYPE: ");
        stringBuilder.append(sandboxType);
        stringBuilder.append("  ITERATIONS: ");
        stringBuilder.append(iterations);
        stringBuilder.append('\n');
        stringBuilder.append('\n');
        for (Map.Entry<CommandType, ExecutionResults> entry: resultsMap.entrySet().stream().sorted(Comparator.comparing(e -> e.getKey().toString())).toList()) {
            stringBuilder.append(entry.getKey());
            stringBuilder.append('\n');
            stringBuilder.append(entry.getValue());
            stringBuilder.append('\n');
        }

        log.info(stringBuilder.toString());
    }
}
