package io.digit.commands;

import java.sql.PreparedStatement;
import java.sql.SQLException;
import java.sql.Statement;

public class Delete implements Command<Boolean> {
    @Override
    public Statement prepareStatement(java.sql.Connection connection, int iteration) throws SQLException {
        return connection.prepareStatement("DELETE FROM TABLE1 where age=?");
    }
    @Override
    public Object execute(Statement statement, int iteration) throws SQLException {
        PreparedStatement preparedStatement = (PreparedStatement) statement;
        preparedStatement.setInt(1, iteration);
        return Boolean.TRUE.equals(preparedStatement.executeUpdate());
    }

    @Override
    public Boolean interpretResults(Object input) {
        return (Boolean) input;
    }
}
