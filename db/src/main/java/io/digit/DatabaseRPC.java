package io.digit;

import io.digit.commands.Command;

public interface DatabaseRPC {
    /**
     * Run the given command.
     */
    Object run(Command command, int iteration);

    /**
     * Is the system ready?
     */
    boolean ready();
}
