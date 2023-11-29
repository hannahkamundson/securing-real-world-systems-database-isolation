package io.digit;

import com.sun.management.OperatingSystemMXBean;

import javax.management.MBeanServerConnection;
import java.io.IOException;
import java.lang.management.ManagementFactory;

public class ExecutionResults {
    private final long totalTime;
    private final long thisCPUTime;

    private final long otherCPUtime;
    private final long totalCPUTime;
    private final double percentCPUTime;

    private ExecutionResults(long totalTime, long thisCPUTime, long otherCPUtime) {
        this.totalTime = totalTime;
        this.thisCPUTime = thisCPUTime;
        this.otherCPUtime = otherCPUtime;
        this.totalCPUTime = thisCPUTime + otherCPUtime;
        this.percentCPUTime = 100 * ((double) totalCPUTime / (double) totalTime);
    }

    @Override
    public String toString() {
        return String.format("total time = %s | this CPU time = %s | other CPU time = %s | total CPU time = %s | CPU overhead = %s)", totalTime, thisCPUTime, otherCPUtime, totalCPUTime, percentCPUTime);
    }

    public static Builder builder() throws IOException {
        return new Builder();
    }

    public static class Builder {
        private final MBeanServerConnection mbsc = ManagementFactory.getPlatformMBeanServer();

        private final OperatingSystemMXBean osMBean = ManagementFactory.newPlatformMXBeanProxy(
                mbsc, ManagementFactory.OPERATING_SYSTEM_MXBEAN_NAME, OperatingSystemMXBean.class);
        private long startTime;
        private long endTime;
        private long cpuStartTime;
        private long cpuEndTime;
        // We need to account for the cpu time from the process
        private long otherCpuStartTime;
        // We need to account for the cpu time from the process
        private long otherCpuEndTime;

        Builder() throws IOException {
        }

        public Builder startTime() {
            this.startTime = System.nanoTime();
            return this;
        }

        public Builder endTime() {
            this.endTime = System.nanoTime();
            return this;
        }

        public Builder cpuStartTime() {
            this.cpuStartTime = osMBean.getProcessCpuTime();
            return this;
        }

        public Builder cpuEndTime() {
            this.cpuEndTime = osMBean.getProcessCpuTime();
            return this;
        }

        public Builder otherCpuStartTime(long cpuTime) {
            this.otherCpuStartTime = cpuTime;
            return this;
        }

        public Builder otherCpuEndTime(long cpuTime) {
            this.otherCpuEndTime = cpuTime;
            return this;
        }

        public ExecutionResults build() {
            return new ExecutionResults(endTime - startTime, cpuEndTime - cpuStartTime, otherCpuEndTime - otherCpuStartTime);
        }
    }
}
