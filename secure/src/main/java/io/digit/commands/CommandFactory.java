package io.digit.commands;

public class CommandFactory {

    public static Command create(String commandName) {
        switch (commandName.toLowerCase()) {
            case "connection":
                return new Connection();
            case "select":
                return new Select();
            default:
                throw new IllegalArgumentException(String.format("The command isn't implemented: %s", commandName));
        }
    }
}
