package io.digit.commands;

import lombok.AllArgsConstructor;
import lombok.Builder;
import lombok.extern.slf4j.Slf4j;

import java.io.Serializable;
import java.sql.Connection;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.sql.Statement;
import java.util.ArrayList;
import java.util.List;

@Slf4j
public class Select implements Command<Select.Results> {
    @Override
    public Statement prepareStatement(Connection connection, int iteration) throws SQLException {
        return connection.createStatement();
    }

    @Override
    public Object execute(Statement statement, int iteration) throws SQLException {
        ResultSet rs = statement.executeQuery("SELECT id, age, gender, name FROM TABLE1;");

        ArrayList<Result> results = new ArrayList<>();



        while (rs.next()) {
            Result result = Result.builder()
                    .id(rs.getInt("id"))
                    .age(rs.getInt("age"))
                    .gender(rs.getString("gender"))
                    .name(rs.getString("name"))
                    .build();
            results.add(result);
        }


        return new Results(results);
    }

    @Builder
    public static class Result implements Serializable {
        private int id;
        private String name;
        private int age;
        private String gender;

        @Override
        public String toString() {
            return String.format("id: %s, name: %s, age: %s, gender: %s", id, name, age, gender);
        }
    }

    @AllArgsConstructor
    public static class Results implements Serializable {
        private final List<Result> results;

        @Override
        public String toString() {
            StringBuilder builder = new StringBuilder();
            for (Result result: results) {
                builder.append(result);
                builder.append('\n');
            }
            return builder.toString();
        }
    }

    @Override
    public Results interpretResults(Object input) {
        return (Results) input;
    }
}
