package io.digit;

import com.sun.management.OperatingSystemMXBean;
import lombok.AllArgsConstructor;
import lombok.Builder;

import javax.management.MBeanServerConnection;
import java.io.IOException;
import java.lang.management.ManagementFactory;

public class ExecutionResults {
    private final long totalTime;
    private final long totalCPUTime;
    private final double percentCPUTime;

    private ExecutionResults(long totalTime, long totalCPUTime) {
        this.totalTime = totalTime;
        this.totalCPUTime = totalCPUTime;
        this.percentCPUTime = 100 * ((double) totalCPUTime / (double) totalTime);
    }

    @Override
    public String toString() {
        return String.format("total time = %s | total CPU time = %s | CPU overhead = %s)", totalTime, totalCPUTime, percentCPUTime);
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

        public ExecutionResults build() {
            return new ExecutionResults(endTime - startTime, cpuEndTime - cpuStartTime);
        }
    }
}
