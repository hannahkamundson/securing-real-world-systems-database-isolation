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
    private final long pid;
    private static final String DATABASE_NAME = "test.db";

    public DatabaseRPCImpl(long pid) {
        this.pid = pid;
    }

    @Override
    public boolean ready() {
        log.info("{}: The database is ready", pid);
        return true;
    }

    @Override
    public Object run(Command command, int iteration) {
        log.info("{}: Running command {}", pid, command.getName());

        // Execute the command
        try(Connection connection = DriverManager.getConnection("jdbc:sqlite:" + DATABASE_NAME)) {
            Statement statement = command.prepareStatement(connection, iteration);
            return command.execute(statement, iteration);
        } catch (SQLException e) {
            throw new RuntimeException(e);
        }
    }
}
