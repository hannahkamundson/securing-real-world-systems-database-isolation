package io.digit.commands;

import java.sql.SQLException;
import java.sql.Statement;

public class DeleteTable implements Command{
    @Override
    public void execute(Statement statement) throws SQLException {
        statement.execute("DROP TABLE TABLE1;");
    }
}
