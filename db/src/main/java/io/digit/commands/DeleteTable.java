package io.digit.commands;

import java.sql.SQLException;
import java.sql.Statement;

public class DeleteTable implements Command<Integer> {
    @Override
    public Statement prepareStatement(java.sql.Connection connection, int iteration) throws SQLException {
        return connection.createStatement();
    }
    @Override
    public Object execute(Statement statement, int iteration) throws SQLException {
        return Integer.valueOf(statement.executeUpdate(String.format("DROP TABLE TABLE%s;", iteration)));
    }

    @Override
    public Integer interpretResults(Object input) {
        return (Integer) input;
    }
}
