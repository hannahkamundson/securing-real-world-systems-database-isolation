package io.digit.commands;

import java.sql.SQLException;
import java.sql.Statement;

public class CreateTable implements Command{
    @Override
    public void execute(Statement statement) throws SQLException {
        statement.execute("CREATE TABLE TABLE1( id int, name varchar(20),age smallint, gender varchar(10) );");
    }
}
