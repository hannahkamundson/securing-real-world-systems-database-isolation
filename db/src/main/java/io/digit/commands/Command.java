package io.digit.commands;

import java.io.Serializable;
import java.sql.Connection;
import java.sql.PreparedStatement;
import java.sql.SQLException;
import java.sql.Statement;

public interface Command<T> extends Serializable {

    Statement prepareStatement(Connection connection, int iteration) throws SQLException;

    Object execute(Statement statement, int iteration) throws SQLException;

    T interpretResults(Object input);

    default String getName() {
        return getClass().getSimpleName();
    }
}
