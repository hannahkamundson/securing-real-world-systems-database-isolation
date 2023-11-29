package io.digit.commands;

public class CommandFactory {

    public static Command<?> create(CommandType type) {
        switch (type) {
            case CONNECTION:
                return new Connection();
            case SELECT:
                return new Select();
            case INSERT:
                return new Insert();
            case DELETE:
                return new Delete();
            case DELETE_TABLE:
                return new DeleteTable();
            case DELETE_DATABASE:
                return new DeleteDatabase();
            case CREATE_TABLE:
                return new CreateTable();
            case CREATE_DATABASE:
                return new CreateDatabase();
            default:
                throw new IllegalArgumentException(String.format("The command isn't implemented"));
        }
    }


}
