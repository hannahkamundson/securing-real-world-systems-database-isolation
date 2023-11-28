package io.digit.commands;

import java.sql.SQLException;
import java.sql.Statement;

public class Insert implements Command{
    @Override
    public void execute(Statement statement) throws SQLException {
        statement.execute("INSERT INTO TABLE1 (id,name,age,gender) values (2,'lucy',17,'female'),(3,'jack',19,'male'),(4,'tom',18,'male');");
    }
}
