package io.digit;

import lombok.Builder;

@Builder
public class ExecutionResults {
    private final long startTime;
    private final long endTime;
    private final int iterations;

    private long totalTime() {
        return endTime - startTime;
    }

    private long averageTime() {
        return totalTime() / iterations;
    }

    @Override
    public String toString() {
        return String.format("iterations: %s | start time: %s | end time: %s | total time: %s | average time: %s", iterations, startTime, endTime, totalTime(), averageTime());
    }
}
