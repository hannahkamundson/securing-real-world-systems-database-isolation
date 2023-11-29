package io.digit.sandbox;

import io.digit.App;
import io.digit.DatabaseRPC;
import io.digit.DatabaseRPCImpl;

import java.io.IOException;

public class NoSandbox implements SandboxCommand {
    @Override
    public DatabaseRPC createDatabaseConnection() throws IOException {
        return new DatabaseRPCImpl(App.PID);
    }

    @Override
    public long getCpuStartTime(DatabaseRPC db) {
        // There is no external process to keep track of
        return 0;
    }

    @Override
    public long getCpuEndTime(DatabaseRPC db) {
        // There is no external process to keep track of
        return 0;
    }
}
