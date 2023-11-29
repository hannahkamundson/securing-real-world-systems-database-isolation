package io.digit.commands;

import java.sql.PreparedStatement;
import java.sql.SQLException;
import java.sql.Statement;

public class Insert implements Command<Integer> {
    @Override
    public Statement prepareStatement(java.sql.Connection connection, int iteration) throws SQLException {
        return connection.prepareStatement("INSERT INTO TABLE1 (id,name,age,gender) values (?,?,?,?);");
    }

    @Override
    public Object execute(Statement statement, int iteration) throws SQLException {
        PreparedStatement prepared = (PreparedStatement) statement;
        prepared.setInt(1, iteration);
        prepared.setString(2, String.format("Lucy%s", iteration));
        prepared.setInt(3, iteration + 20);
        prepared.setString(4, iteration % 2 == 0 ? "female" : "male");
        return Integer.valueOf(prepared.executeUpdate());
    }

    @Override
    public Integer interpretResults(Object input) {
        return (Integer) input;
    }
}
