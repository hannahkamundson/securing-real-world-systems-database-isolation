package io.digit.commands;

import java.sql.SQLException;
import java.sql.Statement;

public class Connection implements Command<Object> {
    @Override
    public Statement prepareStatement(java.sql.Connection connection, int iteration) throws SQLException {
        return connection.createStatement();
    }

    @Override
    public Object execute(Statement statement, int iteration) {
        // No op. We are exclusively testing the connection
        return null;
    }

    @Override
    public Object interpretResults(Object input) {
        // Do nothing
        return null;
    }
}
