package io.digit;

import io.digit.commands.Command;
import io.digit.commands.CommandFactory;
import io.digit.commands.CommandType;
import lombok.extern.slf4j.Slf4j;
import net.sourceforge.argparse4j.ArgumentParsers;
import net.sourceforge.argparse4j.inf.ArgumentParser;
import net.sourceforge.argparse4j.inf.ArgumentParserException;
import net.sourceforge.argparse4j.inf.Namespace;

import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.SQLException;
import java.sql.Statement;

@Slf4j
public class App {
    private static final String DATABASE_NAME = "test.db";

    public static void main(String[] args) {
        ArgumentParser parser = ArgumentParsers.newFor("Sandboxing a database").build()
                .defaultHelp(true)
                .description("Run SQLite JDBC driver with and without a sandbox");
        parser.addArgument("--command", "-c")
                .type(CommandType.class)
                .help("The command you want to run");

        Namespace ns;
        try {
            ns = parser.parseArgs(args);
        } catch (ArgumentParserException e) {
            parser.handleError(e);
            throw new RuntimeException(e);
        }

        // Get the command
        Command command = CommandFactory.create(ns.get("command"));

        log.info("Running command {}", command.getName());

        // Execute the command
        try(Connection connection = DriverManager.getConnection("jdbc:sqlite:" + DATABASE_NAME)) {
            if (connection == null) {
                System.out.println("Let's see");
            }

            Statement statement = connection.createStatement();
            command.execute(statement);
        } catch (SQLException e) {
            throw new RuntimeException(e);
        }
    }
}
