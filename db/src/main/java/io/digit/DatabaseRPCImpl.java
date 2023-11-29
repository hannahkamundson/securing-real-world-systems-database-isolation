package io.digit;

import io.digit.commands.Command;
import io.digit.commands.CommandFactory;
import io.digit.commands.CommandType;
import lombok.extern.slf4j.Slf4j;

import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.SQLException;
import java.sql.Statement;

@Slf4j
public class DatabaseRPCImpl implements DatabaseRPC {
    private static final String DATABASE_NAME = "test.db";

    @Override
    public boolean ready() {
        log.info("The database is ready");
        return true;
    }

    @Override
    public void run(String commandName) {
        run(CommandFactory.create(CommandType.valueOf(commandName)));
    }

    public void run(Command command) {
        log.info("Running command {}", command);

        // Execute the command
        try(Connection connection = DriverManager.getConnection("jdbc:sqlite:" + DATABASE_NAME)) {
            Statement statement = connection.createStatement();
            command.execute(statement);
        } catch (SQLException e) {
            throw new RuntimeException(e);
        }
    }
}
