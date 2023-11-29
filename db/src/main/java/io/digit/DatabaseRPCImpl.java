package io.digit;

import com.sun.management.OperatingSystemMXBean;
import io.digit.commands.Command;
import io.digit.commands.CommandFactory;
import io.digit.commands.CommandType;
import lombok.extern.slf4j.Slf4j;

import javax.management.MBeanServerConnection;
import java.io.IOException;
import java.lang.management.ManagementFactory;
import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.SQLException;
import java.sql.Statement;

@Slf4j
public class DatabaseRPCImpl implements DatabaseRPC {
    private final long pid;
    private static final String DATABASE_NAME = "test.db";

    private final MBeanServerConnection mbsc = ManagementFactory.getPlatformMBeanServer();

    private final OperatingSystemMXBean osMBean = ManagementFactory.newPlatformMXBeanProxy(
            mbsc, ManagementFactory.OPERATING_SYSTEM_MXBEAN_NAME, OperatingSystemMXBean.class);

    public DatabaseRPCImpl(long pid) throws IOException {
        this.pid = pid;
    }

    @Override
    public boolean ready() {
        log.debug("{}: The database is ready", pid);
        return true;
    }

    @Override
    public long getProcessCpuTime() {
        return osMBean.getProcessCpuTime();
    }

    @Override
    public Object run(Command<?> command, int iteration) {
        log.debug("{}: Running command {}", pid, command.getName());

        // Execute the command
        try(Connection connection = DriverManager.getConnection("jdbc:sqlite:" + DATABASE_NAME)) {
            Statement statement = command.prepareStatement(connection, iteration);
            return command.execute(statement, iteration);
        } catch (SQLException e) {
            throw new RuntimeException(e);
        }
    }
}
