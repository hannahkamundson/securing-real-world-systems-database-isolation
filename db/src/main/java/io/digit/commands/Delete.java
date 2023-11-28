package io.digit.commands;

import java.sql.SQLException;
import java.sql.Statement;

public class Delete implements Command{
    @Override
    public void execute(Statement statement) throws SQLException {
        statement.execute("DELETE FROM TABLE1 where age=14;");

    }
}
