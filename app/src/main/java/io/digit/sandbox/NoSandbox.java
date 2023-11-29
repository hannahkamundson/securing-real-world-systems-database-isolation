package io.digit.sandbox;

import io.digit.App;
import io.digit.DatabaseRPC;
import io.digit.DatabaseRPCImpl;

public class NoSandbox implements SandboxCommand {
    @Override
    public DatabaseRPC createDatabaseConnection() {
        return new DatabaseRPCImpl(App.PID);
    }
}
