package io.digit.sandbox;

import io.digit.DatabaseRPC;

import java.io.IOException;

public interface SandboxCommand {
    DatabaseRPC createDatabaseConnection() throws IOException, InterruptedException;

    long getCpuStartTime(DatabaseRPC db);

    long getCpuEndTime(DatabaseRPC db);
}
