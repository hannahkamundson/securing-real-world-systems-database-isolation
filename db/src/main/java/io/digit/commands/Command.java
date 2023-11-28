package io.digit.commands;

import java.sql.SQLException;
import java.sql.Statement;

public interface Command {
    void execute(Statement statement) throws SQLException;

    default String getName() {
        return getClass().getSimpleName();
    }
}
