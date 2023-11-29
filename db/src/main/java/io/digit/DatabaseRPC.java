package io.digit;

public interface DatabaseRPC {
    /**
     * Run the given command.
     */
    void run(String commandName);

    /**
     * Is the system ready?
     */
    boolean ready();
}
