package io.digit.commands;

import java.sql.Statement;

public class Connection implements Command {
    @Override
    public void execute(Statement statement) {
        // No op. We are exclusively testing the connection
    }
}
