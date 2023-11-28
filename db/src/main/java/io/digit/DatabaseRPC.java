package io.digit;

import io.digit.commands.Command;

public interface DatabaseRPC {
    /**
     * Run the given command.
     */
//    void run(Command command);

    /**
     * Is the system ready?
     * @return
     */
    boolean ready();
}
