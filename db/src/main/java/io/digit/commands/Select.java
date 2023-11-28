package io.digit.commands;

import java.sql.SQLException;
import java.sql.Statement;

public class Select implements Command {
    @Override
    public void execute(Statement statement) throws SQLException {
        statement.execute("SELECT * FROM TABLE1;");
    }
}
