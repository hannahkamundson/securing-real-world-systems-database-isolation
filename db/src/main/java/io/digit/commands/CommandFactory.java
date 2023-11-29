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
            case CREATE_TABLE:
                return new CreateTable();
            default:
                throw new IllegalArgumentException("The command isn't implemented");
        }
    }


}
