package io.digit.commands;

public class CommandFactory {

    public static Command create(CommandType type) {
        switch (type) {
            case CONNECTION:
                return new Connection();
            case SELECT:
                return new Select();
            default:
                throw new IllegalArgumentException(String.format("The command isn't implemented"));
        }
    }


}
