package io.digit.commands;

import java.sql.SQLException;
import java.sql.Statement;

public class CreateDatabase implements Command {
    @Override
    public void execute(Statement statement) throws SQLException {
        statement.execute("CREATE DATABASE db;");
    }
}
