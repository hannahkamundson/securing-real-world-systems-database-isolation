package io.digit.commands;

import java.sql.SQLException;
import java.sql.Statement;

public class CreateDatabase implements Command<Boolean> {

    @Override
    public Statement prepareStatement(java.sql.Connection connection, int iteration) throws SQLException {
        return connection.createStatement();
    }

    @Override
    public Object execute(Statement statement, int iteration) throws SQLException {
        return Boolean.TRUE.equals(statement.execute(String.format("CREATE DATABASE test%s.db;", iteration)));
    }

    @Override
    public Boolean interpretResults(Object input) {
        return (Boolean) input;
    }
}
