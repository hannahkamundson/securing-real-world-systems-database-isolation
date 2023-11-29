package io.digit.sandbox;

public class SandboxFactory {
    public static SandboxCommand create(SandboxType type) {
        switch(type) {
            case NONE:
                return new NoSandbox();
            case PROCESS:
                return new ProcessIsolationSandbox();
            default:
                throw new IllegalArgumentException("It must be an allowed sandbox type");
        }
    }

}
